ResizeImage=function(url){
this.url=url;
Node.call(this);
this.outputPort1=null;
this.outputPort2=null;
this.setDimension(100,100);
this.setColor(null);
};
ResizeImage.prototype=new Node;
ResizeImage.prototype.type="ResizeImage";
ResizeImage.prototype.createHTMLElement=function(){
var item=Node.prototype.createHTMLElement.call(this);
if(navigator.appName.toUpperCase()=="MICROSOFT INTERNET EXPLORER"){
this.d=document.createElement("div");
this.d.style.position="absolute";
this.d.style.left="0px";
this.d.style.top="0px";
this.d.style.filter="progid:DXImageTransform.Microsoft.AlphaImageLoader (src='"+this.url+"', sizingMethod='scale')";
item.appendChild(this.d);
}else{
this.img=document.createElement("img");
this.img.style.position="absolute";
this.img.style.left="0px";
this.img.style.top="0px";
this.img.src=this.url;
item.appendChild(this.img);
this.d=document.createElement("div");
this.d.style.position="absolute";
this.d.style.left="0px";
this.d.style.top="0px";
item.appendChild(this.d);
}
item.style.left=this.x+"px";
item.style.top=this.y+"px";
return item;
};
ResizeImage.prototype.setDimension=function(w,h){
Node.prototype.setDimension.call(this,w,h);
if(this.d!=null){
this.d.style.width=this.width+"px";
this.d.style.height=this.height+"px";
}
if(this.img!=null){
this.img.width=this.width;
this.img.height=this.height;
}
if(this.outputPort1!=null){
this.outputPort1.setPosition(this.width+3,this.height/3);
this.outputPort2.setPosition(this.width+3,this.height/3*2);
}
};
ResizeImage.prototype.setWorkflow=function(_2b19){
Node.prototype.setWorkflow.call(this,_2b19);
if(_2b19!=null){
this.outputPort1=new OutputPort();
this.outputPort1.setMaxFanOut(1);
this.outputPort1.setWorkflow(_2b19);
this.outputPort1.setBackgroundColor(new Color(245,115,115));
this.addPort(this.outputPort1,this.width+3,this.height/3);
this.outputPort2=new OutputPort();
this.outputPort2.setMaxFanOut(1);
this.outputPort2.setWorkflow(_2b19);
this.outputPort2.setBackgroundColor(new Color(245,115,115));
this.addPort(this.outputPort2,this.width+3,this.height/3*2);
}
};
