window.onload= resizeSplitWndw;
window.onresize= resizeSplitWndw;
window.onbeforeprint= set_to_print;
window.onafterprint= reset_form;

function resizeSplitWndw(){

var onsr= document.all.item("nsr");
var omainbody= document.all.item("mainbody");

if (omainbody ==null) return;
if (onsr != null){
document.all.mainbody.style.overflow= "auto";
document.all.nsr.style.width= document.body.offsetWidth;
document.all.mainbody.style.width= Math.max(document.body.offsetWidth-4,0);
document.all.mainbody.style.top= document.all.nsr.offsetHeight;
if (document.body.offsetHeight > document.all.nsr.offsetHeight)
document.all.mainbody.style.height= Math.max(document.body.offsetHeight - document.all.nsr.offsetHeight - 4, 0);
else document.all.mainbody.style.height=0;
}
}

function set_to_print(){

var i;
if (window.mainbody)document.all.mainbody.style.height = "auto";

for (i=0; i < document.all.length; i++){
if (document.all[i].tagName == "BODY") {
document.all[i].scroll = "auto";
}
if (document.all[i].tagName == "A") {
document.all[i].outerHTML = "<a href=''>" + document.all[i].innerHTML + "</a>";
}
}
}

function reset_form(){

document.location.reload();
}

function doExpand(paraNum, imageNum) {
	if (paraNum.style.display=="none")           {paraNum.style.display=""; imageNum.src="minus.gif"}
	else {paraNum.style.display="none"; imageNum.src="plus.gif"}
}