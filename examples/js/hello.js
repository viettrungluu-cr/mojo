#!mojo:js_content_handler
// Demonstrate one JS Mojo application connecting to another to emit "hello
// world". To run this application with mojo_shell, set DIR to be the absolute
// path for this directory, then:
//   mojo_shell "file://$DIR/hello.js file://$DIR/world.js"
// Launches the Mojo hello.js application which connects to the application
// URL specified as a Mojo application argument, world.js in this case.

define("main", [
  "console",
  "mojo/public/interfaces/application/service_provider.mojom",
], function(console, sp) {

  function Application(shell, url) {
    this.shell = shell;
    console.log(url + ": Hello");
  }

  Application.prototype.initialize = function(args) {
    if (args.length != 2) {
      console.log("Expected hello.js URL argument");
      return;
    }
    var serviceProvider = new sp.ServiceProvider.proxyClass();
    // TODO(hansmuller): the following step shouldn't be necessary.
    var handle = serviceProvider.getConnection$().messagePipeHandle;
    this.shell.connectToApplication(args[1], handle);
  }

  Application.prototype.acceptConnection = function(url, serviceProvider) {
  }

  return Application;
});
