String.prototype.parseColor=function(){
var color="#";
if(this.slice(0,4)=="rgb("){
var cols=this.slice(4,this.length-1).split(",");
var i=0;
do{
color+=parseInt(cols[i]).toColorPart();
}while(++i<3);
}else{
if(this.slice(0,1)=="#"){
if(this.length==4){
for(var i=1;i<4;i++){
color+=(this.charAt(i)+this.charAt(i)).toLowerCase();
}
}
if(this.length==7){
color=this.toLowerCase();
}
}
}
return (color.length==7?color:(arguments[0]||this));
};
Element.collectTextNodes=function(_3bca){
return $A($(_3bca).childNodes).collect(function(node){
return (node.nodeType==3?node.nodeValue:(node.hasChildNodes()?Element.collectTextNodes(node):""));
}).flatten().join("");
};
Element.collectTextNodesIgnoreClass=function(_3bcc,_3bcd){
return $A($(_3bcc).childNodes).collect(function(node){
return (node.nodeType==3?node.nodeValue:((node.hasChildNodes()&&!Element.hasClassName(node,_3bcd))?Element.collectTextNodesIgnoreClass(node,_3bcd):""));
}).flatten().join("");
};
Element.setContentZoom=function(_3bcf,_3bd0){
_3bcf=$(_3bcf);
_3bcf.setStyle({fontSize:(_3bd0/100)+"em"});
if(navigator.appVersion.indexOf("AppleWebKit")>0){
window.scrollBy(0,0);
}
return _3bcf;
};
Element.getOpacity=function(_3bd1){
return $(_3bd1).getStyle("opacity");
};
Element.setOpacity=function(_3bd2,value){
return $(_3bd2).setStyle({opacity:value});
};
Element.getInlineOpacity=function(_3bd4){
return $(_3bd4).style.opacity||"";
};
Element.forceRerendering=function(_3bd5){
try{
_3bd5=$(_3bd5);
var n=document.createTextNode(" ");
_3bd5.appendChild(n);
_3bd5.removeChild(n);
}
catch(e){
}
};
Array.prototype.call=function(){
var args=arguments;
this.each(function(f){
f.apply(this,args);
});
};
var Effect={_elementDoesNotExistError:{name:"ElementDoesNotExistError",message:"The specified DOM element does not exist, but is required for this effect to operate"},tagifyText:function(_3bd9){
if(typeof Builder=="undefined"){
throw ("Effect.tagifyText requires including script.aculo.us' builder.js library");
}
var _3bda="position:relative";
if(/MSIE/.test(navigator.userAgent)&&!window.opera){
_3bda+=";zoom:1";
}
_3bd9=$(_3bd9);
$A(_3bd9.childNodes).each(function(child){
if(child.nodeType==3){
child.nodeValue.toArray().each(function(_3bdc){
_3bd9.insertBefore(Builder.node("span",{style:_3bda},_3bdc==" "?String.fromCharCode(160):_3bdc),child);
});
Element.remove(child);
}
});
},multiple:function(_3bdd,_3bde){
var _3bdf;
if(((typeof _3bdd=="object")||(typeof _3bdd=="function"))&&(_3bdd.length)){
_3bdf=_3bdd;
}else{
_3bdf=$(_3bdd).childNodes;
}
var _3be0=Object.extend({speed:0.1,delay:0},arguments[2]||{});
var _3be1=_3be0.delay;
$A(_3bdf).each(function(_3be2,index){
new _3bde(_3be2,Object.extend(_3be0,{delay:index*_3be0.speed+_3be1}));
});
},PAIRS:{"slide":["SlideDown","SlideUp"],"blind":["BlindDown","BlindUp"],"appear":["Appear","Fade"]},toggle:function(_3be4,_3be5){
_3be4=$(_3be4);
_3be5=(_3be5||"appear").toLowerCase();
var _3be6=Object.extend({queue:{position:"end",scope:(_3be4.id||"global"),limit:1}},arguments[2]||{});
Effect[_3be4.visible()?Effect.PAIRS[_3be5][1]:Effect.PAIRS[_3be5][0]](_3be4,_3be6);
}};
var Effect2=Effect;
Effect.Transitions={linear:Prototype.K,sinoidal:function(pos){
return (-Math.cos(pos*Math.PI)/2)+0.5;
},reverse:function(pos){
return 1-pos;
},flicker:function(pos){
return ((-Math.cos(pos*Math.PI)/4)+0.75)+Math.random()/4;
},wobble:function(pos){
return (-Math.cos(pos*Math.PI*(9*pos))/2)+0.5;
},pulse:function(pos,_3bec){
_3bec=_3bec||5;
return (Math.round((pos%(1/_3bec))*_3bec)==0?((pos*_3bec*2)-Math.floor(pos*_3bec*2)):1-((pos*_3bec*2)-Math.floor(pos*_3bec*2)));
},none:function(pos){
return 0;
},full:function(pos){
return 1;
}};
Effect.ScopedQueue=Class.create();
Object.extend(Object.extend(Effect.ScopedQueue.prototype,Enumerable),{initialize:function(){
this.effects=[];
this.interval=null;
},_each:function(_3bef){
this.effects._each(_3bef);
},add:function(_3bf0){
var _3bf1=new Date().getTime();
var _3bf2=(typeof _3bf0.options.queue=="string")?_3bf0.options.queue:_3bf0.options.queue.position;
switch(_3bf2){
case "front":
this.effects.findAll(function(e){
return e.state=="idle";
}).each(function(e){
e.startOn+=_3bf0.finishOn;
e.finishOn+=_3bf0.finishOn;
});
break;
case "with-last":
_3bf1=this.effects.pluck("startOn").max()||_3bf1;
break;
case "end":
_3bf1=this.effects.pluck("finishOn").max()||_3bf1;
break;
}
_3bf0.startOn+=_3bf1;
_3bf0.finishOn+=_3bf1;
if(!_3bf0.options.queue.limit||(this.effects.length<_3bf0.options.queue.limit)){
this.effects.push(_3bf0);
}
if(!this.interval){
this.interval=setInterval(this.loop.bind(this),15);
}
},remove:function(_3bf5){
this.effects=this.effects.reject(function(e){
return e==_3bf5;
});
if(this.effects.length==0){
clearInterval(this.interval);
this.interval=null;
}
},loop:function(){
var _3bf7=new Date().getTime();
for(var i=0,len=this.effects.length;i<len;i++){
if(this.effects[i]){
this.effects[i].loop(_3bf7);
}
}
}});
Effect.Queues={instances:$H(),get:function(_3bf9){
if(typeof _3bf9!="string"){
return _3bf9;
}
if(!this.instances[_3bf9]){
this.instances[_3bf9]=new Effect.ScopedQueue();
}
return this.instances[_3bf9];
}};
Effect.Queue=Effect.Queues.get("global");
Effect.DefaultOptions={transition:Effect.Transitions.sinoidal,duration:1,fps:60,sync:false,from:0,to:1,delay:0,queue:"parallel"};
Effect.Base=function(){
};
Effect.Base.prototype={position:null,start:function(_3bfa){
this.options=Object.extend(Object.extend({},Effect.DefaultOptions),_3bfa||{});
this.currentFrame=0;
this.state="idle";
this.startOn=this.options.delay*1000;
this.finishOn=this.startOn+(this.options.duration*1000);
this.event("beforeStart");
if(!this.options.sync){
Effect.Queues.get(typeof this.options.queue=="string"?"global":this.options.queue.scope).add(this);
}
},loop:function(_3bfb){
if(_3bfb>=this.startOn){
if(_3bfb>=this.finishOn){
this.render(1);
this.cancel();
this.event("beforeFinish");
if(this.finish){
this.finish();
}
this.event("afterFinish");
return;
}
var pos=(_3bfb-this.startOn)/(this.finishOn-this.startOn);
var frame=Math.round(pos*this.options.fps*this.options.duration);
if(frame>this.currentFrame){
this.render(pos);
this.currentFrame=frame;
}
}
},render:function(pos){
if(this.state=="idle"){
this.state="running";
this.event("beforeSetup");
if(this.setup){
this.setup();
}
this.event("afterSetup");
}
if(this.state=="running"){
if(this.options.transition){
pos=this.options.transition(pos);
}
pos*=(this.options.to-this.options.from);
pos+=this.options.from;
this.position=pos;
this.event("beforeUpdate");
if(this.update){
this.update(pos);
}
this.event("afterUpdate");
}
},cancel:function(){
if(!this.options.sync){
Effect.Queues.get(typeof this.options.queue=="string"?"global":this.options.queue.scope).remove(this);
}
this.state="finished";
},event:function(_3bff){
if(this.options[_3bff+"Internal"]){
this.options[_3bff+"Internal"](this);
}
if(this.options[_3bff]){
this.options[_3bff](this);
}
},inspect:function(){
var data=$H();
for(property in this){
if(typeof this[property]!="function"){
data[property]=this[property];
}
}
return "#<Effect:"+data.inspect()+",options:"+$H(this.options).inspect()+">";
}};
Effect.Parallel=Class.create();
Object.extend(Object.extend(Effect.Parallel.prototype,Effect.Base.prototype),{initialize:function(_3c01){
this.effects=_3c01||[];
this.start(arguments[1]);
},update:function(_3c02){
this.effects.invoke("render",_3c02);
},finish:function(_3c03){
this.effects.each(function(_3c04){
_3c04.render(1);
_3c04.cancel();
_3c04.event("beforeFinish");
if(_3c04.finish){
_3c04.finish(_3c03);
}
_3c04.event("afterFinish");
});
}});
Effect.Event=Class.create();
Object.extend(Object.extend(Effect.Event.prototype,Effect.Base.prototype),{initialize:function(){
var _3c05=Object.extend({duration:0},arguments[0]||{});
this.start(_3c05);
},update:Prototype.emptyFunction});
Effect.Opacity=Class.create();
Object.extend(Object.extend(Effect.Opacity.prototype,Effect.Base.prototype),{initialize:function(_3c06){
this.element=$(_3c06);
if(!this.element){
throw (Effect._elementDoesNotExistError);
}
if(/MSIE/.test(navigator.userAgent)&&!window.opera&&(!this.element.currentStyle.hasLayout)){
this.element.setStyle({zoom:1});
}
var _3c07=Object.extend({from:this.element.getOpacity()||0,to:1},arguments[1]||{});
this.start(_3c07);
},update:function(_3c08){
this.element.setOpacity(_3c08);
}});
Effect.Move=Class.create();
Object.extend(Object.extend(Effect.Move.prototype,Effect.Base.prototype),{initialize:function(_3c09){
this.element=$(_3c09);
if(!this.element){
throw (Effect._elementDoesNotExistError);
}
var _3c0a=Object.extend({x:0,y:0,mode:"relative"},arguments[1]||{});
this.start(_3c0a);
},setup:function(){
this.element.makePositioned();
this.originalLeft=parseFloat(this.element.getStyle("left")||"0");
this.originalTop=parseFloat(this.element.getStyle("top")||"0");
if(this.options.mode=="absolute"){
this.options.x=this.options.x-this.originalLeft;
this.options.y=this.options.y-this.originalTop;
}
},update:function(_3c0b){
this.element.setStyle({left:Math.round(this.options.x*_3c0b+this.originalLeft)+"px",top:Math.round(this.options.y*_3c0b+this.originalTop)+"px"});
}});
Effect.MoveBy=function(_3c0c,toTop,_3c0e){
return new Effect.Move(_3c0c,Object.extend({x:_3c0e,y:toTop},arguments[3]||{}));
};
Effect.Scale=Class.create();
Object.extend(Object.extend(Effect.Scale.prototype,Effect.Base.prototype),{initialize:function(_3c0f,_3c10){
this.element=$(_3c0f);
if(!this.element){
throw (Effect._elementDoesNotExistError);
}
var _3c11=Object.extend({scaleX:true,scaleY:true,scaleContent:true,scaleFromCenter:false,scaleMode:"box",scaleFrom:100,scaleTo:_3c10},arguments[2]||{});
this.start(_3c11);
},setup:function(){
this.restoreAfterFinish=this.options.restoreAfterFinish||false;
this.elementPositioning=this.element.getStyle("position");
this.originalStyle={};
["top","left","width","height","fontSize"].each(function(k){
this.originalStyle[k]=this.element.style[k];
}.bind(this));
this.originalTop=this.element.offsetTop;
this.originalLeft=this.element.offsetLeft;
var _3c13=this.element.getStyle("font-size")||"100%";
["em","px","%","pt"].each(function(_3c14){
if(_3c13.indexOf(_3c14)>0){
this.fontSize=parseFloat(_3c13);
this.fontSizeType=_3c14;
}
}.bind(this));
this.factor=(this.options.scaleTo-this.options.scaleFrom)/100;
this.dims=null;
if(this.options.scaleMode=="box"){
this.dims=[this.element.offsetHeight,this.element.offsetWidth];
}
if(/^content/.test(this.options.scaleMode)){
this.dims=[this.element.scrollHeight,this.element.scrollWidth];
}
if(!this.dims){
this.dims=[this.options.scaleMode.originalHeight,this.options.scaleMode.originalWidth];
}
},update:function(_3c15){
var _3c16=(this.options.scaleFrom/100)+(this.factor*_3c15);
if(this.options.scaleContent&&this.fontSize){
this.element.setStyle({fontSize:this.fontSize*_3c16+this.fontSizeType});
}
this.setDimensions(this.dims[0]*_3c16,this.dims[1]*_3c16);
},finish:function(_3c17){
if(this.restoreAfterFinish){
this.element.setStyle(this.originalStyle);
}
},setDimensions:function(_3c18,width){
var d={};
if(this.options.scaleX){
d.width=Math.round(width)+"px";
}
if(this.options.scaleY){
d.height=Math.round(_3c18)+"px";
}
if(this.options.scaleFromCenter){
var topd=(_3c18-this.dims[0])/2;
var leftd=(width-this.dims[1])/2;
if(this.elementPositioning=="absolute"){
if(this.options.scaleY){
d.top=this.originalTop-topd+"px";
}
if(this.options.scaleX){
d.left=this.originalLeft-leftd+"px";
}
}else{
if(this.options.scaleY){
d.top=-topd+"px";
}
if(this.options.scaleX){
d.left=-leftd+"px";
}
}
}
this.element.setStyle(d);
}});
Effect.Highlight=Class.create();
Object.extend(Object.extend(Effect.Highlight.prototype,Effect.Base.prototype),{initialize:function(_3c1d){
this.element=$(_3c1d);
if(!this.element){
throw (Effect._elementDoesNotExistError);
}
var _3c1e=Object.extend({startcolor:"#ffff99"},arguments[1]||{});
this.start(_3c1e);
},setup:function(){
if(this.element.getStyle("display")=="none"){
this.cancel();
return;
}
this.oldStyle={};
if(!this.options.keepBackgroundImage){
this.oldStyle.backgroundImage=this.element.getStyle("background-image");
this.element.setStyle({backgroundImage:"none"});
}
if(!this.options.endcolor){
this.options.endcolor=this.element.getStyle("background-color").parseColor("#ffffff");
}
if(!this.options.restorecolor){
this.options.restorecolor=this.element.getStyle("background-color");
}
this._base=$R(0,2).map(function(i){
return parseInt(this.options.startcolor.slice(i*2+1,i*2+3),16);
}.bind(this));
this._delta=$R(0,2).map(function(i){
return parseInt(this.options.endcolor.slice(i*2+1,i*2+3),16)-this._base[i];
}.bind(this));
},update:function(_3c21){
this.element.setStyle({backgroundColor:$R(0,2).inject("#",function(m,v,i){
return m+(Math.round(this._base[i]+(this._delta[i]*_3c21)).toColorPart());
}.bind(this))});
},finish:function(){
this.element.setStyle(Object.extend(this.oldStyle,{backgroundColor:this.options.restorecolor}));
}});
Effect.ScrollTo=Class.create();
Object.extend(Object.extend(Effect.ScrollTo.prototype,Effect.Base.prototype),{initialize:function(_3c25){
this.element=$(_3c25);
this.start(arguments[1]||{});
},setup:function(){
Position.prepare();
var _3c26=Position.cumulativeOffset(this.element);
if(this.options.offset){
_3c26[1]+=this.options.offset;
}
var max=window.innerHeight?window.height-window.innerHeight:document.body.scrollHeight-(document.documentElement.clientHeight?document.documentElement.clientHeight:document.body.clientHeight);
this.scrollStart=Position.deltaY;
this.delta=(_3c26[1]>max?max:_3c26[1])-this.scrollStart;
},update:function(_3c28){
Position.prepare();
window.scrollTo(Position.deltaX,this.scrollStart+(_3c28*this.delta));
}});
Effect.Fade=function(_3c29){
_3c29=$(_3c29);
var _3c2a=_3c29.getInlineOpacity();
var _3c2b=Object.extend({from:_3c29.getOpacity()||1,to:0,afterFinishInternal:function(_3c2c){
if(_3c2c.options.to!=0){
return;
}
_3c2c.element.hide().setStyle({opacity:_3c2a});
}},arguments[1]||{});
return new Effect.Opacity(_3c29,_3c2b);
};
Effect.Appear=function(_3c2d){
_3c2d=$(_3c2d);
var _3c2e=Object.extend({from:(_3c2d.getStyle("display")=="none"?0:_3c2d.getOpacity()||0),to:1,afterFinishInternal:function(_3c2f){
_3c2f.element.forceRerendering();
},beforeSetup:function(_3c30){
_3c30.element.setOpacity(_3c30.options.from).show();
}},arguments[1]||{});
return new Effect.Opacity(_3c2d,_3c2e);
};
Effect.Puff=function(_3c31){
_3c31=$(_3c31);
var _3c32={opacity:_3c31.getInlineOpacity(),position:_3c31.getStyle("position"),top:_3c31.style.top,left:_3c31.style.left,width:_3c31.style.width,height:_3c31.style.height};
return new Effect.Parallel([new Effect.Scale(_3c31,200,{sync:true,scaleFromCenter:true,scaleContent:true,restoreAfterFinish:true}),new Effect.Opacity(_3c31,{sync:true,to:0})],Object.extend({duration:1,beforeSetupInternal:function(_3c33){
Position.absolutize(_3c33.effects[0].element);
},afterFinishInternal:function(_3c34){
_3c34.effects[0].element.hide().setStyle(_3c32);
}},arguments[1]||{}));
};
Effect.BlindUp=function(_3c35){
_3c35=$(_3c35);
_3c35.makeClipping();
return new Effect.Scale(_3c35,0,Object.extend({scaleContent:false,scaleX:false,restoreAfterFinish:true,afterFinishInternal:function(_3c36){
_3c36.element.hide().undoClipping();
}},arguments[1]||{}));
};
Effect.BlindDown=function(_3c37){
_3c37=$(_3c37);
var _3c38=_3c37.getDimensions();
return new Effect.Scale(_3c37,100,Object.extend({scaleContent:false,scaleX:false,scaleFrom:0,scaleMode:{originalHeight:_3c38.height,originalWidth:_3c38.width},restoreAfterFinish:true,afterSetup:function(_3c39){
_3c39.element.makeClipping().setStyle({height:"0px"}).show();
},afterFinishInternal:function(_3c3a){
_3c3a.element.undoClipping();
}},arguments[1]||{}));
};
Effect.SwitchOff=function(_3c3b){
_3c3b=$(_3c3b);
var _3c3c=_3c3b.getInlineOpacity();
return new Effect.Appear(_3c3b,Object.extend({duration:0.4,from:0,transition:Effect.Transitions.flicker,afterFinishInternal:function(_3c3d){
new Effect.Scale(_3c3d.element,1,{duration:0.3,scaleFromCenter:true,scaleX:false,scaleContent:false,restoreAfterFinish:true,beforeSetup:function(_3c3e){
_3c3e.element.makePositioned().makeClipping();
},afterFinishInternal:function(_3c3f){
_3c3f.element.hide().undoClipping().undoPositioned().setStyle({opacity:_3c3c});
}});
}},arguments[1]||{}));
};
Effect.DropOut=function(_3c40){
_3c40=$(_3c40);
var _3c41={top:_3c40.getStyle("top"),left:_3c40.getStyle("left"),opacity:_3c40.getInlineOpacity()};
return new Effect.Parallel([new Effect.Move(_3c40,{x:0,y:100,sync:true}),new Effect.Opacity(_3c40,{sync:true,to:0})],Object.extend({duration:0.5,beforeSetup:function(_3c42){
_3c42.effects[0].element.makePositioned();
},afterFinishInternal:function(_3c43){
_3c43.effects[0].element.hide().undoPositioned().setStyle(_3c41);
}},arguments[1]||{}));
};
Effect.Shake=function(_3c44){
_3c44=$(_3c44);
var _3c45={top:_3c44.getStyle("top"),left:_3c44.getStyle("left")};
return new Effect.Move(_3c44,{x:20,y:0,duration:0.05,afterFinishInternal:function(_3c46){
new Effect.Move(_3c46.element,{x:-40,y:0,duration:0.1,afterFinishInternal:function(_3c47){
new Effect.Move(_3c47.element,{x:40,y:0,duration:0.1,afterFinishInternal:function(_3c48){
new Effect.Move(_3c48.element,{x:-40,y:0,duration:0.1,afterFinishInternal:function(_3c49){
new Effect.Move(_3c49.element,{x:40,y:0,duration:0.1,afterFinishInternal:function(_3c4a){
new Effect.Move(_3c4a.element,{x:-20,y:0,duration:0.05,afterFinishInternal:function(_3c4b){
_3c4b.element.undoPositioned().setStyle(_3c45);
}});
}});
}});
}});
}});
}});
};
Effect.SlideDown=function(_3c4c){
_3c4c=$(_3c4c).cleanWhitespace();
var _3c4d=_3c4c.down().getStyle("bottom");
var _3c4e=_3c4c.getDimensions();
return new Effect.Scale(_3c4c,100,Object.extend({scaleContent:false,scaleX:false,scaleFrom:window.opera?0:1,scaleMode:{originalHeight:_3c4e.height,originalWidth:_3c4e.width},restoreAfterFinish:true,afterSetup:function(_3c4f){
_3c4f.element.makePositioned();
_3c4f.element.down().makePositioned();
if(window.opera){
_3c4f.element.setStyle({top:""});
}
_3c4f.element.makeClipping().setStyle({height:"0px"}).show();
},afterUpdateInternal:function(_3c50){
_3c50.element.down().setStyle({bottom:(_3c50.dims[0]-_3c50.element.clientHeight)+"px"});
},afterFinishInternal:function(_3c51){
_3c51.element.undoClipping().undoPositioned();
_3c51.element.down().undoPositioned().setStyle({bottom:_3c4d});
}},arguments[1]||{}));
};
Effect.SlideUp=function(_3c52){
_3c52=$(_3c52).cleanWhitespace();
var _3c53=_3c52.down().getStyle("bottom");
return new Effect.Scale(_3c52,window.opera?0:1,Object.extend({scaleContent:false,scaleX:false,scaleMode:"box",scaleFrom:100,restoreAfterFinish:true,beforeStartInternal:function(_3c54){
_3c54.element.makePositioned();
_3c54.element.down().makePositioned();
if(window.opera){
_3c54.element.setStyle({top:""});
}
_3c54.element.makeClipping().show();
},afterUpdateInternal:function(_3c55){
_3c55.element.down().setStyle({bottom:(_3c55.dims[0]-_3c55.element.clientHeight)+"px"});
},afterFinishInternal:function(_3c56){
_3c56.element.hide().undoClipping().undoPositioned().setStyle({bottom:_3c53});
_3c56.element.down().undoPositioned();
}},arguments[1]||{}));
};
Effect.Squish=function(_3c57){
return new Effect.Scale(_3c57,window.opera?1:0,{restoreAfterFinish:true,beforeSetup:function(_3c58){
_3c58.element.makeClipping();
},afterFinishInternal:function(_3c59){
_3c59.element.hide().undoClipping();
}});
};
Effect.Grow=function(_3c5a){
_3c5a=$(_3c5a);
var _3c5b=Object.extend({direction:"center",moveTransition:Effect.Transitions.sinoidal,scaleTransition:Effect.Transitions.sinoidal,opacityTransition:Effect.Transitions.full},arguments[1]||{});
var _3c5c={top:_3c5a.style.top,left:_3c5a.style.left,height:_3c5a.style.height,width:_3c5a.style.width,opacity:_3c5a.getInlineOpacity()};
var dims=_3c5a.getDimensions();
var _3c5e,initialMoveY;
var moveX,moveY;
switch(_3c5b.direction){
case "top-left":
_3c5e=initialMoveY=moveX=moveY=0;
break;
case "top-right":
_3c5e=dims.width;
initialMoveY=moveY=0;
moveX=-dims.width;
break;
case "bottom-left":
_3c5e=moveX=0;
initialMoveY=dims.height;
moveY=-dims.height;
break;
case "bottom-right":
_3c5e=dims.width;
initialMoveY=dims.height;
moveX=-dims.width;
moveY=-dims.height;
break;
case "center":
_3c5e=dims.width/2;
initialMoveY=dims.height/2;
moveX=-dims.width/2;
moveY=-dims.height/2;
break;
}
return new Effect.Move(_3c5a,{x:_3c5e,y:initialMoveY,duration:0.01,beforeSetup:function(_3c60){
_3c60.element.hide().makeClipping().makePositioned();
},afterFinishInternal:function(_3c61){
new Effect.Parallel([new Effect.Opacity(_3c61.element,{sync:true,to:1,from:0,transition:_3c5b.opacityTransition}),new Effect.Move(_3c61.element,{x:moveX,y:moveY,sync:true,transition:_3c5b.moveTransition}),new Effect.Scale(_3c61.element,100,{scaleMode:{originalHeight:dims.height,originalWidth:dims.width},sync:true,scaleFrom:window.opera?1:0,transition:_3c5b.scaleTransition,restoreAfterFinish:true})],Object.extend({beforeSetup:function(_3c62){
_3c62.effects[0].element.setStyle({height:"0px"}).show();
},afterFinishInternal:function(_3c63){
_3c63.effects[0].element.undoClipping().undoPositioned().setStyle(_3c5c);
}},_3c5b));
}});
};
Effect.Shrink=function(_3c64){
_3c64=$(_3c64);
var _3c65=Object.extend({direction:"center",moveTransition:Effect.Transitions.sinoidal,scaleTransition:Effect.Transitions.sinoidal,opacityTransition:Effect.Transitions.none},arguments[1]||{});
var _3c66={top:_3c64.style.top,left:_3c64.style.left,height:_3c64.style.height,width:_3c64.style.width,opacity:_3c64.getInlineOpacity()};
var dims=_3c64.getDimensions();
var moveX,moveY;
switch(_3c65.direction){
case "top-left":
moveX=moveY=0;
break;
case "top-right":
moveX=dims.width;
moveY=0;
break;
case "bottom-left":
moveX=0;
moveY=dims.height;
break;
case "bottom-right":
moveX=dims.width;
moveY=dims.height;
break;
case "center":
moveX=dims.width/2;
moveY=dims.height/2;
break;
}
return new Effect.Parallel([new Effect.Opacity(_3c64,{sync:true,to:0,from:1,transition:_3c65.opacityTransition}),new Effect.Scale(_3c64,window.opera?1:0,{sync:true,transition:_3c65.scaleTransition,restoreAfterFinish:true}),new Effect.Move(_3c64,{x:moveX,y:moveY,sync:true,transition:_3c65.moveTransition})],Object.extend({beforeStartInternal:function(_3c69){
_3c69.effects[0].element.makePositioned().makeClipping();
},afterFinishInternal:function(_3c6a){
_3c6a.effects[0].element.hide().undoClipping().undoPositioned().setStyle(_3c66);
}},_3c65));
};
Effect.Pulsate=function(_3c6b){
_3c6b=$(_3c6b);
var _3c6c=arguments[1]||{};
var _3c6d=_3c6b.getInlineOpacity();
var _3c6e=_3c6c.transition||Effect.Transitions.sinoidal;
var _3c6f=function(pos){
return _3c6e(1-Effect.Transitions.pulse(pos,_3c6c.pulses));
};
_3c6f.bind(_3c6e);
return new Effect.Opacity(_3c6b,Object.extend(Object.extend({duration:2,from:0,afterFinishInternal:function(_3c71){
_3c71.element.setStyle({opacity:_3c6d});
}},_3c6c),{transition:_3c6f}));
};
Effect.Fold=function(_3c72){
_3c72=$(_3c72);
var _3c73={top:_3c72.style.top,left:_3c72.style.left,width:_3c72.style.width,height:_3c72.style.height};
_3c72.makeClipping();
return new Effect.Scale(_3c72,5,Object.extend({scaleContent:false,scaleX:false,afterFinishInternal:function(_3c74){
new Effect.Scale(_3c72,1,{scaleContent:false,scaleY:false,afterFinishInternal:function(_3c75){
_3c75.element.hide().undoClipping().setStyle(_3c73);
}});
}},arguments[1]||{}));
};
Effect.Morph=Class.create();
Object.extend(Object.extend(Effect.Morph.prototype,Effect.Base.prototype),{initialize:function(_3c76){
this.element=$(_3c76);
if(!this.element){
throw (Effect._elementDoesNotExistError);
}
var _3c77=Object.extend({style:{}},arguments[1]||{});
if(typeof _3c77.style=="string"){
if(_3c77.style.indexOf(":")==-1){
var _3c78="",selector="."+_3c77.style;
$A(document.styleSheets).reverse().each(function(_3c79){
if(_3c79.cssRules){
cssRules=_3c79.cssRules;
}else{
if(_3c79.rules){
cssRules=_3c79.rules;
}
}
$A(cssRules).reverse().each(function(rule){
if(selector==rule.selectorText){
_3c78=rule.style.cssText;
throw $break;
}
});
if(_3c78){
throw $break;
}
});
this.style=_3c78.parseStyle();
_3c77.afterFinishInternal=function(_3c7b){
_3c7b.element.addClassName(_3c7b.options.style);
_3c7b.transforms.each(function(_3c7c){
if(_3c7c.style!="opacity"){
_3c7b.element.style[_3c7c.style.camelize()]="";
}
});
};
}else{
this.style=_3c77.style.parseStyle();
}
}else{
this.style=$H(_3c77.style);
}
this.start(_3c77);
},setup:function(){
function parseColor(color){
if(!color||["rgba(0, 0, 0, 0)","transparent"].include(color)){
color="#ffffff";
}
color=color.parseColor();
return $R(0,2).map(function(i){
return parseInt(color.slice(i*2+1,i*2+3),16);
});
}
this.transforms=this.style.map(function(pair){
var _3c80=pair[0].underscore().dasherize(),value=pair[1],unit=null;
if(value.parseColor("#zzzzzz")!="#zzzzzz"){
value=value.parseColor();
unit="color";
}else{
if(_3c80=="opacity"){
value=parseFloat(value);
if(/MSIE/.test(navigator.userAgent)&&!window.opera&&(!this.element.currentStyle.hasLayout)){
this.element.setStyle({zoom:1});
}
}else{
if(Element.CSS_LENGTH.test(value)){
var _3c81=value.match(/^([\+\-]?[0-9\.]+)(.*)$/),value=parseFloat(_3c81[1]),unit=(_3c81.length==3)?_3c81[2]:null;
}
}
}
var _3c82=this.element.getStyle(_3c80);
return $H({style:_3c80,originalValue:unit=="color"?parseColor(_3c82):parseFloat(_3c82||0),targetValue:unit=="color"?parseColor(value):value,unit:unit});
}.bind(this)).reject(function(_3c83){
return ((_3c83.originalValue==_3c83.targetValue)||(_3c83.unit!="color"&&(isNaN(_3c83.originalValue)||isNaN(_3c83.targetValue))));
});
},update:function(_3c84){
var style=$H(),value=null;
this.transforms.each(function(_3c86){
value=_3c86.unit=="color"?$R(0,2).inject("#",function(m,v,i){
return m+(Math.round(_3c86.originalValue[i]+(_3c86.targetValue[i]-_3c86.originalValue[i])*_3c84)).toColorPart();
}):_3c86.originalValue+Math.round(((_3c86.targetValue-_3c86.originalValue)*_3c84)*1000)/1000+_3c86.unit;
style[_3c86.style]=value;
});
this.element.setStyle(style);
}});
Effect.Transform=Class.create();
Object.extend(Effect.Transform.prototype,{initialize:function(_3c8a){
this.tracks=[];
this.options=arguments[1]||{};
this.addTracks(_3c8a);
},addTracks:function(_3c8b){
_3c8b.each(function(track){
var data=$H(track).values().first();
this.tracks.push($H({ids:$H(track).keys().first(),effect:Effect.Morph,options:{style:data}}));
}.bind(this));
return this;
},play:function(){
return new Effect.Parallel(this.tracks.map(function(track){
var _3c8f=[$(track.ids)||$$(track.ids)].flatten();
return _3c8f.map(function(e){
return new track.effect(e,Object.extend({sync:true},track.options));
});
}).flatten(),this.options);
}});
Element.CSS_PROPERTIES=$w("backgroundColor backgroundPosition borderBottomColor borderBottomStyle "+"borderBottomWidth borderLeftColor borderLeftStyle borderLeftWidth "+"borderRightColor borderRightStyle borderRightWidth borderSpacing "+"borderTopColor borderTopStyle borderTopWidth bottom clip color "+"fontSize fontWeight height left letterSpacing lineHeight "+"marginBottom marginLeft marginRight marginTop markerOffset maxHeight "+"maxWidth minHeight minWidth opacity outlineColor outlineOffset "+"outlineWidth paddingBottom paddingLeft paddingRight paddingTop "+"right textIndent top width wordSpacing zIndex");
Element.CSS_LENGTH=/^(([\+\-]?[0-9\.]+)(em|ex|px|in|cm|mm|pt|pc|\%))|0$/;
String.prototype.parseStyle=function(){
var _3c91=Element.extend(document.createElement("div"));
_3c91.innerHTML="<div style=\""+this+"\"></div>";
var style=_3c91.down().style,styleRules=$H();
Element.CSS_PROPERTIES.each(function(_3c93){
if(style[_3c93]){
styleRules[_3c93]=style[_3c93];
}
});
if(/MSIE/.test(navigator.userAgent)&&!window.opera&&this.indexOf("opacity")>-1){
styleRules.opacity=this.match(/opacity:\s*((?:0|1)?(?:\.\d*)?)/)[1];
}
return styleRules;
};
Element.morph=function(_3c94,style){
new Effect.Morph(_3c94,Object.extend({style:style},arguments[2]||{}));
return _3c94;
};
["setOpacity","getOpacity","getInlineOpacity","forceRerendering","setContentZoom","collectTextNodes","collectTextNodesIgnoreClass","morph"].each(function(f){
Element.Methods[f]=Element[f];
});
Element.Methods.visualEffect=function(_3c97,_3c98,_3c99){
s=_3c98.gsub(/_/,"-").camelize();
effect_class=s.charAt(0).toUpperCase()+s.substring(1);
new Effect[effect_class](_3c97,_3c99);
return $(_3c97);
};
Element.addMethods();
