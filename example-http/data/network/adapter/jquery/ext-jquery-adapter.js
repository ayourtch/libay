Ext={};
window["undefined"]=window["undefined"];
Ext.apply=function(o,c,_3){
if(_3){
Ext.apply(o,_3);
}
if(o&&c&&typeof c=="object"){
for(var p in c){
o[p]=c[p];
}
}
return o;
};
(function(){
var _5=0;
var ua=navigator.userAgent.toLowerCase();
var _7=document.compatMode=="CSS1Compat",_8=ua.indexOf("opera")>-1,_9=(/webkit|khtml/).test(ua),_a=ua.indexOf("msie")>-1,_b=ua.indexOf("msie 7")>-1,_c=!_9&&ua.indexOf("gecko")>-1,_d=_a&&!_7,_e=(ua.indexOf("windows")!=-1||ua.indexOf("win32")!=-1),_f=(ua.indexOf("macintosh")!=-1||ua.indexOf("mac os x")!=-1),_10=(ua.indexOf("linux")!=-1),_11=window.location.href.toLowerCase().indexOf("https")===0;
if(_a&&!_b){
try{
document.execCommand("BackgroundImageCache",false,true);
}
catch(e){
}
}
Ext.apply(Ext,{isStrict:_7,isSecure:_11,isReady:false,enableGarbageCollector:true,enableListenerCollection:false,SSL_SECURE_URL:"javascript:false",BLANK_IMAGE_URL:"http:/"+"/extjs.com/s.gif",emptyFn:function(){
},applyIf:function(o,c){
if(o&&c){
for(var p in c){
if(typeof o[p]=="undefined"){
o[p]=c[p];
}
}
}
return o;
},addBehaviors:function(o){
if(!Ext.isReady){
Ext.onReady(function(){
Ext.addBehaviors(o);
});
return;
}
var _16={};
for(var b in o){
var _18=b.split("@");
if(_18[1]){
var s=_18[0];
if(!_16[s]){
_16[s]=Ext.select(s);
}
_16[s].on(_18[1],o[b]);
}
}
_16=null;
},id:function(el,_1b){
_1b=_1b||"ext-gen";
el=Ext.getDom(el);
var id=_1b+(++_5);
return el?(el.id?el.id:(el.id=id)):id;
},extend:function(){
var io=function(o){
for(var m in o){
this[m]=o[m];
}
};
return function(sb,sp,_22){
if(typeof sp=="object"){
_22=sp;
sp=sb;
sb=function(){
sp.apply(this,arguments);
};
}
var F=function(){
},sbp,spp=sp.prototype;
F.prototype=spp;
sbp=sb.prototype=new F();
sbp.constructor=sb;
sb.superclass=spp;
if(spp.constructor==Object.prototype.constructor){
spp.constructor=sp;
}
sb.override=function(o){
Ext.override(sb,o);
};
sbp.override=io;
Ext.override(sb,_22);
return sb;
};
}(),override:function(_27,_28){
if(_28){
var p=_27.prototype;
for(var _2a in _28){
p[_2a]=_28[_2a];
}
}
},namespace:function(){
var a=arguments,o=null,i,j,d,rt;
for(i=0;i<a.length;++i){
d=a[i].split(".");
rt=d[0];
eval("if (typeof "+rt+" == \"undefined\"){"+rt+" = {};} o = "+rt+";");
for(j=1;j<d.length;++j){
o[d[j]]=o[d[j]]||{};
o=o[d[j]];
}
}
},urlEncode:function(o){
if(!o){
return "";
}
var buf=[];
for(var key in o){
var ov=o[key];
var _35=typeof ov;
if(_35=="undefined"){
buf.push(encodeURIComponent(key),"=&");
}else{
if(_35!="function"&&_35!="object"){
buf.push(encodeURIComponent(key),"=",encodeURIComponent(ov),"&");
}else{
if(ov instanceof Array){
for(var i=0,len=ov.length;i<len;i++){
buf.push(encodeURIComponent(key),"=",encodeURIComponent(ov[i]===undefined?"":ov[i]),"&");
}
}
}
}
}
buf.pop();
return buf.join("");
},urlDecode:function(_38,_39){
if(!_38||!_38.length){
return {};
}
var obj={};
var _3b=_38.split("&");
var _3c,_3d,_3e;
for(var i=0,len=_3b.length;i<len;i++){
_3c=_3b[i].split("=");
_3d=decodeURIComponent(_3c[0]);
_3e=decodeURIComponent(_3c[1]);
if(_39!==true){
if(typeof obj[_3d]=="undefined"){
obj[_3d]=_3e;
}else{
if(typeof obj[_3d]=="string"){
obj[_3d]=[obj[_3d]];
obj[_3d].push(_3e);
}else{
obj[_3d].push(_3e);
}
}
}else{
obj[_3d]=_3e;
}
}
return obj;
},each:function(_41,fn,_43){
if(typeof _41.length=="undefined"||typeof _41=="string"){
_41=[_41];
}
for(var i=0,len=_41.length;i<len;i++){
if(fn.call(_43||_41[i],_41[i],i,_41)===false){
return i;
}
}
},combine:function(){
var as=arguments,l=as.length,r=[];
for(var i=0;i<l;i++){
var a=as[i];
if(a instanceof Array){
r=r.concat(a);
}else{
if(a.length!==undefined&&!a.substr){
r=r.concat(Array.prototype.slice.call(a,0));
}else{
r.push(a);
}
}
}
return r;
},escapeRe:function(s){
return s.replace(/([.*+?^${}()|[\]\/\\])/g,"\\$1");
},callback:function(cb,_4d,_4e,_4f){
if(typeof cb=="function"){
if(_4f){
cb.defer(_4f,_4d,_4e||[]);
}else{
cb.apply(_4d,_4e||[]);
}
}
},getDom:function(el){
if(!el){
return null;
}
return el.dom?el.dom:(typeof el=="string"?document.getElementById(el):el);
},getCmp:function(id){
return Ext.ComponentMgr.get(id);
},num:function(v,_53){
if(typeof v!="number"){
return _53;
}
return v;
},destroy:function(){
for(var i=0,a=arguments,len=a.length;i<len;i++){
var as=a[i];
if(as){
if(as.dom){
as.removeAllListeners();
as.remove();
continue;
}
if(typeof as.purgeListeners=="function"){
as.purgeListeners();
}
if(typeof as.destroy=="function"){
as.destroy();
}
}
}
},type:function(o){
if(o===undefined||o===null){
return false;
}
if(o.htmlElement){
return "element";
}
var t=typeof o;
if(t=="object"&&o.nodeName){
switch(o.nodeType){
case 1:
return "element";
case 3:
return (/\S/).test(o.nodeValue)?"textnode":"whitespace";
}
}
if(t=="object"||t=="function"){
switch(o.constructor){
case Array:
return "array";
case RegExp:
return "regexp";
}
if(typeof o.length=="number"&&typeof o.item=="function"){
return "nodelist";
}
}
return t;
},isEmpty:function(v,_5b){
return v===null||v===undefined||(!_5b?v==="":false);
},isOpera:_8,isSafari:_9,isIE:_a,isIE7:_b,isGecko:_c,isBorderBox:_d,isWindows:_e,isLinux:_10,isMac:_f,useShims:((_a&&!_b)||(_c&&_f))});
})();
Ext.namespace("Ext","Ext.util","Ext.grid","Ext.dd","Ext.tree","Ext.data","Ext.form","Ext.menu","Ext.state","Ext.lib","Ext.layout","Ext.app","Ext.ux");
Ext.apply(Function.prototype,{createCallback:function(){
var _5c=arguments;
var _5d=this;
return function(){
return _5d.apply(window,_5c);
};
},createDelegate:function(obj,_5f,_60){
var _61=this;
return function(){
var _62=_5f||arguments;
if(_60===true){
_62=Array.prototype.slice.call(arguments,0);
_62=_62.concat(_5f);
}else{
if(typeof _60=="number"){
_62=Array.prototype.slice.call(arguments,0);
var _63=[_60,0].concat(_5f);
Array.prototype.splice.apply(_62,_63);
}
}
return _61.apply(obj||window,_62);
};
},defer:function(_64,obj,_66,_67){
var fn=this.createDelegate(obj,_66,_67);
if(_64){
return setTimeout(fn,_64);
}
fn();
return 0;
},createSequence:function(fcn,_6a){
if(typeof fcn!="function"){
return this;
}
var _6b=this;
return function(){
var _6c=_6b.apply(this||window,arguments);
fcn.apply(_6a||this||window,arguments);
return _6c;
};
},createInterceptor:function(fcn,_6e){
if(typeof fcn!="function"){
return this;
}
var _6f=this;
return function(){
fcn.target=this;
fcn.method=_6f;
if(fcn.apply(_6e||this||window,arguments)===false){
return;
}
return _6f.apply(this||window,arguments);
};
}});
Ext.applyIf(String,{escape:function(_70){
return _70.replace(/('|\\)/g,"\\$1");
},leftPad:function(val,_72,ch){
var _74=new String(val);
if(ch===null||ch===undefined||ch===""){
ch=" ";
}
while(_74.length<_72){
_74=ch+_74;
}
return _74;
},format:function(_75){
var _76=Array.prototype.slice.call(arguments,1);
return _75.replace(/\{(\d+)\}/g,function(m,i){
return _76[i];
});
}});
String.prototype.toggle=function(_79,_7a){
return this==_79?_7a:_79;
};
Ext.applyIf(Number.prototype,{constrain:function(min,max){
return Math.min(Math.max(this,min),max);
}});
Ext.applyIf(Array.prototype,{indexOf:function(o){
for(var i=0,len=this.length;i<len;i++){
if(this[i]==o){
return i;
}
}
return -1;
},remove:function(o){
var _81=this.indexOf(o);
if(_81!=-1){
this.splice(_81,1);
}
}});
Date.prototype.getElapsed=function(_82){
return Math.abs((_82||new Date()).getTime()-this.getTime());
};
if(typeof jQuery=="undefined"){
throw "Unable to load Ext, jQuery not found.";
}
(function(){
var _1;
Ext.lib.Dom={getViewWidth:function(_2){
return _2?Math.max(jQuery(document).width(),jQuery(window).width()):jQuery(window).width();
},getViewHeight:function(_3){
return _3?Math.max(jQuery(document).height(),jQuery(window).height()):jQuery(window).height();
},isAncestor:function(p,c){
p=Ext.getDom(p);
c=Ext.getDom(c);
if(!p||!c){
return false;
}
if(p.contains&&!Ext.isSafari){
return p.contains(c);
}else{
if(p.compareDocumentPosition){
return !!(p.compareDocumentPosition(c)&16);
}else{
var _6=c.parentNode;
while(_6){
if(_6==p){
return true;
}else{
if(!_6.tagName||_6.tagName.toUpperCase()=="HTML"){
return false;
}
}
_6=_6.parentNode;
}
return false;
}
}
},getRegion:function(el){
return Ext.lib.Region.getRegion(el);
},getY:function(el){
return this.getXY(el)[1];
},getX:function(el){
return this.getXY(el)[0];
},getXY:function(el){
var p,pe,b,_e,bd=document.body;
el=Ext.getDom(el);
if(el.getBoundingClientRect){
b=el.getBoundingClientRect();
_e=fly(document).getScroll();
return [b.left+_e.left,b.top+_e.top];
}
var x=0,y=0;
p=el;
var _12=fly(el).getStyle("position")=="absolute";
while(p){
x+=p.offsetLeft;
y+=p.offsetTop;
if(!_12&&fly(p).getStyle("position")=="absolute"){
_12=true;
}
if(Ext.isGecko){
pe=fly(p);
var bt=parseInt(pe.getStyle("borderTopWidth"),10)||0;
var bl=parseInt(pe.getStyle("borderLeftWidth"),10)||0;
x+=bl;
y+=bt;
if(p!=el&&pe.getStyle("overflow")!="visible"){
x+=bl;
y+=bt;
}
}
p=p.offsetParent;
}
if(Ext.isSafari&&_12){
x-=bd.offsetLeft;
y-=bd.offsetTop;
}
if(Ext.isGecko&&!_12){
var dbd=fly(bd);
x+=parseInt(dbd.getStyle("borderLeftWidth"),10)||0;
y+=parseInt(dbd.getStyle("borderTopWidth"),10)||0;
}
p=el.parentNode;
while(p&&p!=bd){
if(!(Ext.isOpera&&p.tagName!="TR"&&fly(p).getStyle("display")!="inline")){
x-=p.scrollLeft;
y-=p.scrollTop;
}
p=p.parentNode;
}
return [x,y];
},setXY:function(el,xy){
el=Ext.fly(el,"_setXY");
el.position();
var pts=el.translatePoints(xy);
if(xy[0]!==false){
el.dom.style.left=pts.left+"px";
}
if(xy[1]!==false){
el.dom.style.top=pts.top+"px";
}
},setX:function(el,x){
this.setXY(el,[x,false]);
},setY:function(el,y){
this.setXY(el,[false,y]);
}};
function fly(el){
if(!_1){
_1=new Ext.Element.Flyweight();
}
_1.dom=el;
return _1;
}
Ext.lib.Event={getPageX:function(e){
e=e.browserEvent||e;
return e.pageX;
},getPageY:function(e){
e=e.browserEvent||e;
return e.pageY;
},getXY:function(e){
e=e.browserEvent||e;
return [e.pageX,e.pageY];
},getTarget:function(e){
return e.target;
},on:function(el,_23,fn,_25,_26){
jQuery(el).bind(_23,fn);
},un:function(el,_28,fn){
jQuery(el).unbind(_28,fn);
},purgeElement:function(el){
jQuery(el).unbind();
},preventDefault:function(e){
e=e.browserEvent||e;
if(e.preventDefault){
e.preventDefault();
}else{
e.returnValue=false;
}
},stopPropagation:function(e){
e=e.browserEvent||e;
if(e.stopPropagation){
e.stopPropagation();
}else{
e.cancelBubble=true;
}
},stopEvent:function(e){
this.preventDefault(e);
this.stopPropagation(e);
},onAvailable:function(id,fn,_30){
var _31=new Date();
var f=function(){
if(_31.getElapsed()>10000){
clearInterval(iid);
}
var el=document.getElementById(id);
if(el){
clearInterval(iid);
fn.call(_30||window,el);
}
};
var iid=setInterval(f,50);
},resolveTextNode:function(_35){
if(_35&&3==_35.nodeType){
return _35.parentNode;
}else{
return _35;
}
},getRelatedTarget:function(ev){
ev=ev.browserEvent||ev;
var t=ev.relatedTarget;
if(!t){
if(ev.type=="mouseout"){
t=ev.toElement;
}else{
if(ev.type=="mouseover"){
t=ev.fromElement;
}
}
}
return this.resolveTextNode(t);
}};
Ext.lib.Ajax=function(){
var _38=function(cb){
return function(xhr,_3b){
if((_3b=="error"||_3b=="timeout")&&cb.failure){
cb.failure.call(cb.scope||window,{responseText:xhr.responseText,responseXML:xhr.responseXML,argument:cb.argument});
}else{
if(cb.success){
cb.success.call(cb.scope||window,{responseText:xhr.responseText,responseXML:xhr.responseXML,argument:cb.argument});
}
}
};
};
return {request:function(_3c,uri,cb,_3f,_40){
var o={type:_3c,url:uri,data:_3f,timeout:cb.timeout,complete:_38(cb)};
if(_40){
if(_40.xmlData){
o.data=_40.xmlData;
o.processData=false;
o.type="POST";
o.contentType="text/xml";
}
if(_40.headers){
o.beforeSend=function(xhr){
var hs=_40.headers;
for(var h in hs){
if(hs.hasOwnProperty(h)){
xhr.setRequestHeader(h,hs[h]);
}
}
};
}
}
jQuery.ajax(o);
},formRequest:function(_45,uri,cb,_48,_49,_4a){
jQuery.ajax({type:Ext.getDom(_45).method||"POST",url:uri,data:jQuery(_45).formSerialize()+(_48?"&"+_48:""),timeout:cb.timeout,complete:_38(cb)});
},isCallInProgress:function(_4b){
return false;
},abort:function(_4c){
return false;
},serializeForm:function(_4d){
return jQuery(_4d.dom||_4d).formSerialize();
}};
}();
Ext.lib.Anim=function(){
var _4e=function(cb,_50){
var _51=true;
return {stop:function(_52){
},isAnimated:function(){
return _51;
},proxyCallback:function(){
_51=false;
Ext.callback(cb,_50);
}};
};
return {scroll:function(el,_54,_55,_56,cb,_58){
var _59=_4e(cb,_58);
el=Ext.getDom(el);
el.scrollLeft=_54.scroll.to[0];
el.scrollTop=_54.scroll.to[1];
_59.proxyCallback();
return _59;
},motion:function(el,_5b,_5c,_5d,cb,_5f){
return this.run(el,_5b,_5c,_5d,cb,_5f);
},color:function(el,_61,_62,_63,cb,_65){
var _66=_4e(cb,_65);
_66.proxyCallback();
return _66;
},run:function(el,_68,_69,_6a,cb,_6c,_6d){
var _6e=_4e(cb,_6c),e=Ext.fly(el,"_animrun");
var o={};
for(var k in _68){
if(_68[k].from){
if(k!="points"){
e.setStyle(k,_68[k].from);
}
}
switch(k){
case "points":
var by,pts;
e.position();
if(by=_68.points.by){
var xy=e.getXY();
pts=e.translatePoints([xy[0]+by[0],xy[1]+by[1]]);
}else{
pts=e.translatePoints(_68.points.to);
}
o.left=pts.left;
o.top=pts.top;
if(!parseInt(e.getStyle("left"),10)){
e.setLeft(0);
}
if(!parseInt(e.getStyle("top"),10)){
e.setTop(0);
}
if(_68.points.from){
e.setXY(_68.points.from);
}
break;
case "width":
o.width=_68.width.to;
break;
case "height":
o.height=_68.height.to;
break;
case "opacity":
o.opacity=_68.opacity.to;
break;
case "left":
o.left=_68.left.to;
break;
case "top":
o.top=_68.top.to;
break;
default:
o[k]=_68[k].to;
break;
}
}
jQuery(el).animate(o,_69*1000,undefined,_6e.proxyCallback);
return _6e;
}};
}();
Ext.lib.Region=function(t,r,b,l){
this.top=t;
this[1]=t;
this.right=r;
this.bottom=b;
this.left=l;
this[0]=l;
};
Ext.lib.Region.prototype={contains:function(_79){
return (_79.left>=this.left&&_79.right<=this.right&&_79.top>=this.top&&_79.bottom<=this.bottom);
},getArea:function(){
return ((this.bottom-this.top)*(this.right-this.left));
},intersect:function(_7a){
var t=Math.max(this.top,_7a.top);
var r=Math.min(this.right,_7a.right);
var b=Math.min(this.bottom,_7a.bottom);
var l=Math.max(this.left,_7a.left);
if(b>=t&&r>=l){
return new Ext.lib.Region(t,r,b,l);
}else{
return null;
}
},union:function(_7f){
var t=Math.min(this.top,_7f.top);
var r=Math.max(this.right,_7f.right);
var b=Math.max(this.bottom,_7f.bottom);
var l=Math.min(this.left,_7f.left);
return new Ext.lib.Region(t,r,b,l);
},adjust:function(t,l,b,r){
this.top+=t;
this.left+=l;
this.right+=r;
this.bottom+=b;
return this;
}};
Ext.lib.Region.getRegion=function(el){
var p=Ext.lib.Dom.getXY(el);
var t=p[1];
var r=p[0]+el.offsetWidth;
var b=p[1]+el.offsetHeight;
var l=p[0];
return new Ext.lib.Region(t,r,b,l);
};
Ext.lib.Point=function(x,y){
if(x instanceof Array){
y=x[1];
x=x[0];
}
this.x=this.right=this.left=this[0]=x;
this.y=this.top=this.bottom=this[1]=y;
};
Ext.lib.Point.prototype=new Ext.lib.Region();
if(Ext.isIE){
function fnCleanUp(){
var p=Function.prototype;
delete p.createSequence;
delete p.defer;
delete p.createDelegate;
delete p.createCallback;
delete p.createInterceptor;
window.detachEvent("onunload",fnCleanUp);
}
window.attachEvent("onunload",fnCleanUp);
}
})();
