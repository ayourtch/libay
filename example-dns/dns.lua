-- Prosody IM
-- This file is included with Prosody IM. It has modifications,
-- which are hereby placed in the public domain.


-- todo: quick (default) header generation
-- todo: nxdomain, error handling
-- todo: cache results of encodeName


-- reference: http://tools.ietf.org/html/rfc1035
-- reference: http://tools.ietf.org/html/rfc1876 (LOC)



local _, windows = pcall(require, "util.windows");
local is_windows = (_ and windows) or os.getenv("WINDIR");

local coroutine, io, math, string, table =
      coroutine, io, math, string, table;

local ipairs, next, pairs, print, setmetatable, tostring, assert, error, unpack, select, type=
      ipairs, next, pairs, print, setmetatable, tostring, assert, error, unpack, select, type;

local ztact = { -- public domain 20080404 lua@ztact.com
	get = function(parent, ...)
		local len = select('#', ...);
		for i=1,len do
			parent = parent[select(i, ...)];
			if parent == nil then break; end
		end
		return parent;
	end;
	set = function(parent, ...)
		local len = select('#', ...);
		local key, value = select(len-1, ...);
		local cutpoint, cutkey;

		for i=1,len-2 do
			local key = select (i, ...)
			local child = parent[key]

			if value == nil then
				if child == nil then
					return;
				elseif next(child, next(child)) then
					cutpoint = nil; cutkey = nil;
				elseif cutpoint == nil then
					cutpoint = parent; cutkey = key;
				end
			elseif child == nil then
				child = {};
				parent[key] = child;
			end
			parent = child
		end

		if value == nil and cutpoint then
			cutpoint[cutkey] = nil;
		else
			parent[key] = value;
			return value;
		end
	end;
};
local get, set = ztact.get, ztact.set;

local default_timeout = 15;

-------------------------------------------------- module dns
module('dns')
local dns = _M;


-- dns type & class codes ------------------------------ dns type & class codes


local append = table.insert


local function highbyte(i)    -- - - - - - - - - - - - - - - - - - -  highbyte
	return (i-(i%0x100))/0x100;
end


local function augment (t)    -- - - - - - - - - - - - - - - - - - - -  augment
	local a = {};
	for i,s in pairs(t) do
		a[i] = s;
		a[s] = s;
		a[string.lower(s)] = s;
	end
	return a;
end


local function encode (t)    -- - - - - - - - - - - - - - - - - - - - -  encode
	local code = {};
	for i,s in pairs(t) do
		local word = string.char(highbyte(i), i%0x100);
		code[i] = word;
		code[s] = word;
		code[string.lower(s)] = word;
	end
	return code;
end


dns.types = {
	'A', 'NS', 'MD', 'MF', 'CNAME', 'SOA', 'MB', 'MG', 'MR', 'NULL', 'WKS',
	'PTR', 'HINFO', 'MINFO', 'MX', 'TXT',
	[ 28] = 'AAAA', [ 29] = 'LOC',   [ 33] = 'SRV',
	[252] = 'AXFR', [253] = 'MAILB', [254] = 'MAILA', [255] = '*' };


dns.classes = { 'IN', 'CS', 'CH', 'HS', [255] = '*' };


dns.type      = augment (dns.types);
dns.class     = augment (dns.classes);
dns.typecode  = encode  (dns.types);
dns.classcode = encode  (dns.classes);



local function standardize(qname, qtype, qclass)    -- - - - - - - standardize
	if string.byte(qname, -1) ~= 0x2E then qname = qname..'.';  end
	qname = string.lower(qname);
	return qname, dns.type[qtype or 'A'], dns.class[qclass or 'IN'];
end


local function prune(rrs, time, soft)    -- - - - - - - - - - - - - - -  prune
	for i,rr in pairs(rrs) do
		if rr.tod then
			-- rr.tod = rr.tod - 50    -- accelerated decripitude
			rr.ttl = math.floor(rr.tod - time);
			if rr.ttl <= 0 then
				table.remove(rrs, i);
				return prune(rrs, time, soft); -- Re-iterate
			end
		elseif soft == 'soft' then    -- What is this?  I forget!
			assert(rr.ttl == 0);
			rrs[i] = nil;
		end
	end
end


-- metatables & co. ------------------------------------------ metatables & co.


local resolver = {};
resolver.__index = resolver;

resolver.timeout = default_timeout;

local function default_rr_tostring(rr)
	local rr_val = rr.type and rr[rr.type:lower()];
	if type(rr_val) ~= "string" then
		return "<UNKNOWN RDATA TYPE>";
	end
	return rr_val;
end

local special_tostrings = {
	LOC = resolver.LOC_tostring;
	MX  = function (rr)
		return string.format('%2i %s', rr.pref, rr.mx);
	end;
	SRV = function (rr)
		local s = rr.srv;
		return string.format('%5d %5d %5d %s', s.priority, s.weight, s.port, s.target);
	end;
};

local rr_metatable = {};   -- - - - - - - - - - - - - - - - - - -  rr_metatable
function rr_metatable.__tostring(rr)
	local rr_string = (special_tostrings[rr.type] or default_rr_tostring)(rr);
        local rr_name = rr.name or "NIL"
	return string.format('%2s %-5s %6i %-28s %s', rr.class, rr.type, rr.ttl, rr.name, rr_string);
end


local rrs_metatable = {};    -- - - - - - - - - - - - - - - - - -  rrs_metatable
function rrs_metatable.__tostring(rrs)
	local t = {};
	for i,rr in pairs(rrs) do
		append(t, tostring(rr)..'\n');
	end
	return table.concat(t);
end


local cache_metatable = {};    -- - - - - - - - - - - - - - - -  cache_metatable
function cache_metatable.__tostring(cache)
	local time = FIXME;
	local t = {};
	for class,types in pairs(cache) do
		for type,names in pairs(types) do
			for name,rrs in pairs(names) do
				prune(rrs, time);
				append(t, tostring(rrs));
			end
		end
	end
	return table.concat(t);
end


function resolver:new()    -- - - - - - - - - - - - - - - - - - - - - resolver
	local r = { active = {}, cache = {}, unsorted = {} };
	setmetatable(r, resolver);
	setmetatable(r.cache, cache_metatable);
	setmetatable(r.unsorted, { __mode = 'kv' });
	return r;
end


-- packet layer -------------------------------------------------- packet layer


function dns.random(...)    -- - - - - - - - - - - - - - - - - - -  dns.random
	math.randomseed(math.floor(10000) % 0x100000000);
	dns.random = math.random;
	return dns.random(...);
end


local function encodeHeader(o)    -- - - - - - - - - - - - - - -  encodeHeader
	o = o or {};
	o.id = o.id or dns.random(0, 0xffff); -- 16b	(random) id

	o.rd = o.rd or 1;		--  1b  1 recursion desired
	o.tc = o.tc or 0;		--  1b	1 truncated response
	o.aa = o.aa or 0;		--  1b	1 authoritative response
	o.opcode = o.opcode or 0;	--  4b	0 query
				--  1 inverse query
				--	2 server status request
				--	3-15 reserved
	o.qr = o.qr or 0;		--  1b	0 query, 1 response

	o.rcode = o.rcode or 0;	--  4b  0 no error
				--	1 format error
				--	2 server failure
				--	3 name error
				--	4 not implemented
				--	5 refused
				--	6-15 reserved
	o.z = o.z  or 0;		--  3b  0 resvered
	o.ra = o.ra or 0;		--  1b  1 recursion available

	o.qdcount = o.qdcount or 1;	-- 16b	number of question RRs
	o.ancount = o.ancount or 0;	-- 16b	number of answers RRs
	o.nscount = o.nscount or 0;	-- 16b	number of nameservers RRs
	o.arcount = o.arcount or 0;	-- 16b  number of additional RRs

	-- string.char() rounds, so prevent roundup with -0.4999
	local header = string.char(
		highbyte(o.id), o.id %0x100,
		o.rd + 2*o.tc + 4*o.aa + 8*o.opcode + 128*o.qr,
		o.rcode + 16*o.z + 128*o.ra,
		highbyte(o.qdcount),  o.qdcount %0x100,
		highbyte(o.ancount),  o.ancount %0x100,
		highbyte(o.nscount),  o.nscount %0x100,
		highbyte(o.arcount),  o.arcount %0x100
	);

	return header, o.id;
end


local function encodeName(name)    -- - - - - - - - - - - - - - - - encodeName
	local t = {};
	for part in string.gmatch(name, '[^.]+') do
		append(t, string.char(string.len(part)));
		append(t, part);
	end
	append(t, string.char(0));
	return table.concat(t);
end


local function encodeQuestion(qname, qtype, qclass)    -- - - - encodeQuestion
	qname  = encodeName(qname);
	qtype  = dns.typecode[qtype or 'a'];
	qclass = dns.classcode[qclass or 'in'];
	return qname..qtype..qclass;
end


function resolver:byte(len)    -- - - - - - - - - - - - - - - - - - - - - byte
	len = len or 1;
	local offset = self.offset;
	local last = offset + len - 1;
	if last > #self.packet then
		error(string.format('out of bounds: %i>%i', last, #self.packet));
	end
	self.offset = offset + len;
	return string.byte(self.packet, offset, last);
end


function resolver:word()    -- - - - - - - - - - - - - - - - - - - - - -  word
	local b1, b2 = self:byte(2);
	return 0x100*b1 + b2;
end


function resolver:dword ()    -- - - - - - - - - - - - - - - - - - - - -  dword
	local b1, b2, b3, b4 = self:byte(4);
	--print('dword', b1, b2, b3, b4);
	return 0x1000000*b1 + 0x10000*b2 + 0x100*b3 + b4;
end


function resolver:sub(len)    -- - - - - - - - - - - - - - - - - - - - - - sub
	len = len or 1;
	local s = string.sub(self.packet, self.offset, self.offset + len - 1);
	self.offset = self.offset + len;
	return s;
end


function resolver:header(force)    -- - - - - - - - - - - - - - - - - - header
	local id = self:word();
	--print(string.format(':header  id  %x', id));
	if not self.active[id] and not force then return nil; end

	local h = { id = id };

	local b1, b2 = self:byte(2);

	h.rd      = b1 %2;
	h.tc      = b1 /2%2;
	h.aa      = b1 /4%2;
	h.opcode  = b1 /8%16;
	h.qr      = b1 /128;

	h.rcode   = b2 %16;
	h.z       = b2 /16%8;
	h.ra      = b2 /128;

	h.qdcount = self:word();
	h.ancount = self:word();
	h.nscount = self:word();
	h.arcount = self:word();

	for k,v in pairs(h) do h[k] = v-v%1; end

	return h;
end


function resolver:name()    -- - - - - - - - - - - - - - - - - - - - - -  name
	local remember, pointers = nil, 0;
	local len = self:byte();
	local n = {};
	if len == 0 then return "." end -- Root label
	while len > 0 do
		if len >= 0xc0 then    -- name is "compressed"
			pointers = pointers + 1;
			if pointers >= 20 then error('dns error: 20 pointers'); end;
			local offset = ((len-0xc0)*0x100) + self:byte();
			remember = remember or self.offset;
			self.offset = offset + 1;    -- +1 for lua
		else    -- name is not compressed
			append(n, self:sub(len)..'.');
		end
		len = self:byte();
	end
	self.offset = remember or self.offset;
	return table.concat(n);
end


function resolver:question()    -- - - - - - - - - - - - - - - - - -  question
	local q = {};
	q.name  = self:name();
	q.type  = dns.type[self:word()];
	q.class = dns.class[self:word()];
	return q;
end


function resolver:A(rr)    -- - - - - - - - - - - - - - - - - - - - - - - -  A
	local b1, b2, b3, b4 = self:byte(4);
	rr.a = string.format('%i.%i.%i.%i', b1, b2, b3, b4);
	rr.val = rr.a
end

function resolver:AAAA(rr)
	local addr = {};
	for i = 1, rr.rdlength, 2 do
		local b1, b2 = self:byte(2);
		table.insert(addr, ("%02x%02x"):format(b1, b2));
	end
	addr = table.concat(addr, ":"):gsub("%f[%x]0+(%x)","%1");
	local zeros = {};
	for item in addr:gmatch(":[0:]+:") do
		table.insert(zeros, item)
	end
	if #zeros == 0 then
		rr.aaaa = addr;
		rr.val = rr.aaaa
		return
	elseif #zeros > 1 then
		table.sort(zeros, function(a, b) return #a > #b end);
	end
	rr.aaaa = addr:gsub(zeros[1], "::", 1):gsub("^0::", "::"):gsub("::0$", "::");
	rr.val = rr.aaaa
end

function resolver:CNAME(rr)    -- - - - - - - - - - - - - - - - - - - -  CNAME
	rr.cname = self:name();
	rr.val = rr.cname
end


function resolver:MX(rr)    -- - - - - - - - - - - - - - - - - - - - - - -  MX
	rr.pref = self:word();
	rr.mx   = self:name();
	rr.val = rr.pref .. " " .. rr.mx
end


function resolver:LOC_nibble_power()    -- - - - - - - - - -  LOC_nibble_power
	local b = self:byte();
	--print('nibbles', ((b-(b%0x10))/0x10), (b%0x10));
	return ((b-(b%0x10))/0x10) * (10^(b%0x10));
end


function resolver:LOC(rr)    -- - - - - - - - - - - - - - - - - - - - - -  LOC
	rr.version = self:byte();
	if rr.version == 0 then
		rr.loc           = rr.loc or {};
		rr.loc.size      = self:LOC_nibble_power();
		rr.loc.horiz_pre = self:LOC_nibble_power();
		rr.loc.vert_pre  = self:LOC_nibble_power();
		rr.loc.latitude  = self:dword();
		rr.loc.longitude = self:dword();
		rr.loc.altitude  = self:dword();
	end
	rr.val = rr.loc
end


local function LOC_tostring_degrees(f, pos, neg)    -- - - - - - - - - - - - -
	f = f - 0x80000000;
	if f < 0 then pos = neg; f = -f; end
	local deg, min, msec;
	msec = f%60000;
	f    = (f-msec)/60000;
	min  = f%60;
	deg = (f-min)/60;
	return string.format('%3d %2d %2.3f %s', deg, min, msec/1000, pos);
end


function resolver.LOC_tostring(rr)    -- - - - - - - - - - - - -  LOC_tostring
	local t = {};

	--[[
	for k,name in pairs { 'size', 'horiz_pre', 'vert_pre', 'latitude', 'longitude', 'altitude' } do
		append(t, string.format('%4s%-10s: %12.0f\n', '', name, rr.loc[name]));
	end
	--]]

	append(t, string.format(
		'%s    %s    %.2fm %.2fm %.2fm %.2fm',
		LOC_tostring_degrees (rr.loc.latitude, 'N', 'S'),
		LOC_tostring_degrees (rr.loc.longitude, 'E', 'W'),
		(rr.loc.altitude - 10000000) / 100,
		rr.loc.size / 100,
		rr.loc.horiz_pre / 100,
		rr.loc.vert_pre / 100
	));

	return table.concat(t);
end


function resolver:NS(rr)    -- - - - - - - - - - - - - - - - - - - - - - -  NS
	rr.ns = self:name();
	rr.val = rr.ns
end


function resolver:SOA(rr)    -- - - - - - - - - - - - - - - - - - - - - -  SOA
end


function resolver:SRV(rr)    -- - - - - - - - - - - - - - - - - - - - - -  SRV
	  rr.srv = {};
	  rr.srv.priority = self:word();
	  rr.srv.weight   = self:word();
	  rr.srv.port     = self:word();
	  rr.srv.target   = self:name();
	  rr.val = string.format("%d %d %d %s", rr.srv.priority, rr.srv.weight, rr.srv.port, rr.srv.target)
end

function resolver:PTR(rr)
	rr.ptr = self:name();
	rr.val = rr.ptr
end

function resolver:TXT(rr)    -- - - - - - - - - - - - - - - - - - - - - -  TXT
	rr.txt = {};
        table.insert(rr.txt, self:sub (self:byte()));
	rr.val = rr.txt[1]
end


function resolver:rr()    -- - - - - - - - - - - - - - - - - - - - - - - -  rr
	local rr = {};
	setmetatable(rr, rr_metatable);
	rr.name     = self:name(self);
	rr.type     = dns.type[self:word()] or rr.type;
	rr.class    = dns.class[self:word()] or rr.class;
	rr.ttl      = 0x10000*self:word() + self:word();
	rr.rdlength = self:word();

	if rr.ttl <= 0 then
		rr.tod = self.time + 30;
	else
		rr.tod = self.time + rr.ttl;
	end

	local remember = self.offset;
	local rr_parser = self[dns.type[rr.type]];
	if rr_parser then rr_parser(self, rr); end
	self.offset = remember;
	rr.rdata = self:sub(rr.rdlength);
	return rr;
end


function resolver:rrs (count)    -- - - - - - - - - - - - - - - - - - - - - rrs
	local rrs = {};
	for i = 1,count do append(rrs, self:rr()); end
	return rrs;
end


function resolver:decode(packet, force)    -- - - - - - - - - - - - - - decode
	self.packet, self.offset = packet, 1;
	local header = self:header(force);
	if not header then return nil; end
	local response = { header = header };

	response.question = {};
	local offset = self.offset;
	for i = 1,response.header.qdcount do
		append(response.question, self:question());
	end
	response.question.raw = string.sub(self.packet, offset, self.offset - 1);

	if not force then
		if not self.active[response.header.id] or not self.active[response.header.id][response.question.raw] then
			self.active[response.header.id] = nil;
			return nil;
		end
	end

	response.answer     = self:rrs(response.header.ancount);
	response.authority  = self:rrs(response.header.nscount);
	response.additional = self:rrs(response.header.arcount);

	return response;
end


--print ---------------------------------------------------------------- print


local hints = {    -- - - - - - - - - - - - - - - - - - - - - - - - - - - hints
	qr = { [0]='query', 'response' },
	opcode = { [0]='query', 'inverse query', 'server status request' },
	aa = { [0]='non-authoritative', 'authoritative' },
	tc = { [0]='complete', 'truncated' },
	rd = { [0]='recursion not desired', 'recursion desired' },
	ra = { [0]='recursion not available', 'recursion available' },
	z  = { [0]='(reserved)' },
	rcode = { [0]='no error', 'format error', 'server failure', 'name error', 'not implemented' },

	type = dns.type,
	class = dns.class
};


local function hint(p, s)    -- - - - - - - - - - - - - - - - - - - - - - hint
	return (hints[s] and hints[s][p[s]]) or '';
end


function resolver.print(response)    -- - - - - - - - - - - - - resolver.print
	for s,s in pairs { 'id', 'qr', 'opcode', 'aa', 'tc', 'rd', 'ra', 'z',
						'rcode', 'qdcount', 'ancount', 'nscount', 'arcount' } do
		print( string.format('%-30s', 'header.'..s), response.header[s], hint(response.header, s) );
	end

	for i,question in ipairs(response.question) do
		print(string.format ('question[%i].name         ', i), question.name);
		print(string.format ('question[%i].type         ', i), question.type);
		print(string.format ('question[%i].class        ', i), question.class);
	end

	local common = { name=1, type=1, class=1, ttl=1, rdlength=1, rdata=1 };
	local tmp;
	for s,s in pairs({'answer', 'authority', 'additional'}) do
		for i,rr in pairs(response[s]) do
			for j,t in pairs({ 'name', 'type', 'class', 'ttl', 'rdlength' }) do
				tmp = string.format('%s[%i].%s', s, i, t);
				print(string.format('%-30s', tmp), rr[t], hint(rr, t));
			end
			for j,t in pairs(rr) do
				if not common[j] then
					tmp = string.format('%s[%i].%s', s, i, j);
					print(string.format('%-30s  %s', tostring(tmp), tostring(t)));
				end
			end
		end
	end
end


-- module api ------------------------------------------------------ module api


function dns.resolver ()    -- - - - - - - - - - - - - - - - - - - - - resolver
	-- this function seems to be redundant with resolver.new ()

	local r = { active = {}, cache = {}, unsorted = {}, wanted = {}, best_server = 1 };
	setmetatable (r, resolver);
	setmetatable (r.cache, cache_metatable);
	setmetatable (r.unsorted, { __mode = 'kv' });
	return r;
end

local _resolver = dns.resolver();
dns._resolver = _resolver;

function dns.lookup(...)    -- - - - - - - - - - - - - - - - - - - - -  lookup
	return _resolver:lookup(...);
end

function dns.tohostname(...)
	return _resolver:tohostname(...);
end

function dns.purge(...)    -- - - - - - - - - - - - - - - - - - - - - -  purge
	return _resolver:purge(...);
end

function dns.peek(...)    -- - - - - - - - - - - - - - - - - - - - - - -  peek
	return _resolver:peek(...);
end

function dns.query(...)    -- - - - - - - - - - - - - - - - - - - - - -  query
	return _resolver:query(...);
end

function dns.feed(...)    -- - - - - - - - - - - - - - - - - - - - - - -  feed
	return _resolver:feed(...);
end

function dns.cancel(...)  -- - - - - - - - - - - - - - - - - - - - - -  cancel
	return _resolver:cancel(...);
end

function dns.settimeout(...)
	return _resolver:settimeout(...);
end

function dns.cache()
	return _resolver.cache;
end

function dns.socket_wrapper_set(...)    -- - - - - - - - -  socket_wrapper_set
	return _resolver:socket_wrapper_set(...);
end

return dns;
