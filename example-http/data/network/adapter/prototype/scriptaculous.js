var Scriptaculous={Version:"1.7.0",require:function(_2b1e){
document.write("<script type=\"text/javascript\" src=\""+_2b1e+"\"></script>");
},load:function(){
if((typeof Prototype=="undefined")||(typeof Element=="undefined")||(typeof Element.Methods=="undefined")||parseFloat(Prototype.Version.split(".")[0]+"."+Prototype.Version.split(".")[1])<1.5){
throw ("script.aculo.us requires the Prototype JavaScript framework >= 1.5.0");
}
$A(document.getElementsByTagName("script")).findAll(function(s){
return (s.src&&s.src.match(/scriptaculous\.js(\?.*)?$/));
}).each(function(s){
var path=s.src.replace(/scriptaculous\.js(\?.*)?$/,"");
var _2b22=s.src.match(/\?.*load=([a-z,]*)/);
(_2b22?_2b22[1]:"builder,effects,dragdrop,controls,slider").split(",").each(function(_2b23){
Scriptaculous.require(path+_2b23+".js");
});
});
}};
Scriptaculous.load();
