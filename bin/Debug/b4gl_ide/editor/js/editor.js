// set up ace editor
var editor = ace.edit("editor");
editor.setTheme("ace/theme/twilight");
editor.getSession().setMode("ace/mode/b4GL");
var ws = new WebSocket("ws://localhost:8080",1);

var filename = "";

// get text out of editor
function returnEditorText(sock) {
    ws.send(btoa(editor.getValue()));
}

//
function loadFile(name, content) {
  editor.setValue(atob(content));
  filename = name;
  humane.log("Opened "+name.replace(/^.*[\\\/]/, ''));
}

window.onload = function() {
  humane.baseCls = 'humane-flatty'
  humane.log("Welcome Back!");
}

/*toastr.options = {
  "closeButton": false,
  "debug": false,
  "newestOnTop": false,
  "progressBar": false,
  "positionClass": "toast-top-center",
  "preventDuplicates": true,
  "onclick": null,
  "showDuration": "150",
  "hideDuration": "500",
  "timeOut": "5000",
  "extendedTimeOut": "1000",
  "showEasing": "swing",
  "hideEasing": "linear",
  "showMethod": "fadeIn",
  "hideMethod": "fadeOut"
}
*/
