#!mojo:js_content_handler
// See hello.js.
define("main", ["console"], function(console) {
  function Application(shell, url) {
    console.log(url + ": World");
  }

  Application.prototype.initialize = function(args) {
  }

  Application.prototype.acceptConnection = function(url, serviceProvider) {
  }

  return Application;
});
