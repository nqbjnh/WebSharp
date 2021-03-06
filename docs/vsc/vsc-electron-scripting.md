# Example Scripting Electron from CLR Application

## ScriptingElectron Application

This document will take you through creating an `Electron Dotnet` application for VS Code ("ScriptingElectron") and will explain the basic `Nodejs` integration points from C# code to interact with `Electron` from your CLR application.

In this walkthrough, you'll begin with setting up a basic `Electron Dotnet` application that calls a C# routine from JavaScript code.  The C# function that is generated from the template formats a string and returns that formatted string back to the calling javascript function to be written out to the console from JavaScript code.

Once this is up and running we will begin modifying the application to script `Electron` functions directly from the C# application instead.  

## Prerequisites
You need [node.js](https://nodejs.org/en/) installed and available in your $PATH.
   * Plugins will not be used in this example.

   * `electron-dotnet` needs to be built.  The easiest way is to use the provided `make` files available in the WebSharp base directory.  
   
      * [Windows Prerequisites](https://github.com/xamarin/WebSharp/tree/master/electron-dotnet#building-on-windows)
   
     ``` bash
     # Windows Visual Studio 2015 Command Line Prompt 
     nmake /f Makefile.win buildRelease
     ```

      * [Mac Prerequisites](https://github.com/xamarin/WebSharp/tree/master/electron-dotnet#building-on-osx)
     ``` bash
     # Mac OSX terminal with XCode tools available for build.
     make setup  # only needs to be run the first time
     make build
     ```
     
## Getting Started      
  * Install [.NET Core.](https://www.microsoft.com/net/core)
  * Install the [C# extension from the VS Code Marketplace](https://marketplace.visualstudio.com/items?itemName=ms-vscode.csharp).

## Generate a `Electron Dotnet` Application
The simplest way to add a new `Electron DotNet` application for VS Code consumption is through adding a project. A project will have all the boot strap files available to run the `Electron` application.  For more details you can look at the [Electron Quick Start](https://github.com/electron/electron-quick-start).

We have written a Yeoman generator to help get you started. Install Yeoman and the Yeoman Electron Dotnet generator that provides different templates for creating new applications:

``` bash
npm install -g yo path-to-WebSharp-directory\Tools\generator-electron-dotnet
yo electron-dotnet
```
You will be presented with three different project types. For this example, we will pick `Node.js .NET Scripting`.  

![The electron-dotnet generator](./screenshots/yogen-mac.png)ß

Once the type of application project is selected the generator will present you with a series of questions so that the generator can generate the application for you.  Fill in the answers as shown in the following screen shot.

![The electron-dotnet generator questions](./screenshots/yogen-mac-ask-electron.PNG)

Hit enter to start generating the application structure.

![The electron-dotnet generator install](./screenshots/yogen-mac-install-electron.png)

This will install the dependencies automatically and when done you will have the application generated.

![The electron-dotnet generator finish](./screenshots/yogen-mac-finish-electron.PNG)


## The Structure of an application
After running, the generated application should have the following structure:

```
.
|--- .eslintrc.json
|--- .gitignore
|--- .vscode                           // VS Code integration
     |--- launch.json                  // Launch Configurations
     |--- settings.json
|--- .vscodeignore
|--- electron-dotnet-quickstart.md
|--- index.html                       // Html to be displayed in the app window
|--- jsconfig.json
|--- main.js                          // Defines the electron main process
|--- node_modules
     |--- All the node files used to run the electron application
|--- package.json                     // Various project metadata
|--- README.md
|--- renderer.js                      // Required in index.html and executed in the renderer process for that window 
|--- src                              // sources
     |--- scriptingelectron.js              // javascript code implementation

```

Let's go through the purpose of some of these files and explain what they do:

### The application manifest: package.json
The format of `package.json` is exactly the same as that of [Node’s modules](https://docs.npmjs.com/getting-started/using-a-package.json), and the script specified by the `main` field is the startup script of your app, which will run the main process.

* It serves as documentation for what packages your project depends on.
* It allows you to specify the versions of a package that your project can use using [semantic versioning rules](https://docs.npmjs.com/getting-started/semantic-versioning).
* Makes your build reproducable which means that its way easier to share with other developers.

``` json
{
    "name": "scriptingelectron",
    "displayName": "ScriptingElectron",
    "description": "Example Scripting Electron from CLR Application",
    "version": "0.0.1",
    "publisher": "Xamarin",
    "main": "main.js",
    "scripts": {
        "start": "electron ."
    },
    "devDependencies": {
        "electron": "^1.4.0"
    }
}
```

As you can see above the information that was filled in from the template questions has been placed in this file.

Electron is listed as one of the devDependencies and should have been installed when the template was created.  

A `start` script has already been defined in the `scripts` section that can easily be run as the target of a `npm run` command and is the default target for `npm start`.  In this case since we are creating an Electron application is set to run `electron .`.

### Start up script of our app: main.js

The `main.js` should create windows and handle system events.

``` js
// app is the Module to control application life.
// BrowserWindow is the Module to create native browser window.
const {app, BrowserWindow} = require('electron')

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;

function createWindow () {
  // Create the browser window.
  mainWindow = new BrowserWindow({width: 600, height: 400})

  // and load the index.html of the app.
  mainWindow.loadURL(`file://${__dirname}/index.html`)

  // Open the DevTools.
  //mainWindow.webContents.openDevTools()

  // Emitted when the window is closed.
  mainWindow.on('closed', function () {
    // Dereference the window object, usually you would store windows
    // in an array if your app supports multi windows, this is the time
    // when you should delete the corresponding element.
    mainWindow = null
  })
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', function () {
  // On OS X it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', function () {
  // On OS X it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (mainWindow === null) {
    createWindow()
  }
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.

```

### Showing Information: index.html
The web page you want to show is defined in the static html file `index.html`.  This file is loaded from the `main.js` file referenced above in the `createWindow()` method.

``` js
 // and load the index.html of the app.
  mainWindow.loadURL(`file://${__dirname}/index.html`)
```

The static html file is just a normal file containing html elements that will be displayed in Electron's browser page.

``` html
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>ScriptingElectron</title>
  </head>
  <body>
    <h1>ScriptingElectron</h1>
    <!-- All of the Node.js APIs are available in this renderer process. -->
    We are using node <script>document.write(process.versions.node)</script>,
    Chromium <script>document.write(process.versions.chrome)</script>,
    and Electron <script>document.write(process.versions.electron)</script>.
  </body>

  <script>
    // You can also require other files to run in this process
    require('./renderer.js')
  </script>
</html>

```

### Renderer Process: renderer.js

Since Electron uses Chromium for displaying web pages, Chromium’s multi-process architecture is also used. Each web page in Electron runs in its own process, which is called the renderer process.

In normal browsers, web pages usually run in a sandboxed environment and are not allowed access to native resources. Electron users, however, have the power to use Node.js APIs in web pages allowing lower level operating system interactions.

``` js
// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.
var hello = require("./src/scriptingelectron.js");

hello.sayHello();
```

We require the electron-dotnet.js generated file `"./src/scriptingelectron.js"` which is the .NET and Node.js in-process implementation. To print out the `Hello` message we will execute the exported method from that file `sayHello` which displays the message to the console.

When we get to debugging you will see why it is broken up this way.

> :bulb: We will not be going into the differences of Electron's Main and Renderer processes but for more informaiton you can start [here](http://electron.atom.io/docs/tutorial/quick-start/#differences-between-main-process-and-renderer-process) 


### Generated Code

The generated application's code is in the `src` directory.  Depending on the project template selected there is an implementation of each type of application integration point generated.  When we selected the `Node.js .NET Scripting` we will have a total of one src file:

``` bash
.
|--- src                              // sources
     |--- scriptingelectron.js                // javascript code implementation
```

#### electron-dotnet.js: .NET and Node.js in-process implementation: scriptingelectron.js

``` js
var dotnet = require('electron-dotnet');

var hello = dotnet.func('async (input) => { return ".NET welcomes " + input.ToString(); }');

//Make method externaly visible this will be referenced in the renderer.js file
exports.sayHello = arg => {
	hello('Electron', function (error, result) {
		if (error) throw error;
		console.log(result);
	});
}
```

The first two lines of the file enables scripting C# from a Node.js and then creates a C# `hello` function for us.

For more detailed information about scripting C# from a Node.js process please reference the [How to: integrate C# code into Node.js code](
https://github.com/xamarin/WebSharp/tree/master/electron-dotnet#how-to-integrate-c-code-into-nodejs-code
) section of the documentation.

What we do need to point out is the code:

``` js
//Make method externaly visible this will be referenced in the renderer.js file
exports.sayHello = arg => {
	hello('Electron', function (error, result) {
		if (error) throw error;
		console.log(result);
	});
}
``` 

We will be referencing this code from Electron's Renderer process which can be found in the [`renderer.js` file mentioned above](#renderer-process-rendererjs). To do this we will export a function called `sayHello` so that it can be referenced in other processes.

At the bottom of our [index.html](#showing-information-indexhtml) we require `renderer.js` which loads this file into Electrons's [Renderer](http://electron.atom.io/docs/tutorial/quick-start/#renderer-process) process.  

``` js
  <script>
    // You can also require other files to run in this process
    require('./renderer.js')
  </script>
```

### Miscellaneous files
* .vscode/launch.json - [Defines Debugger launching targets](./vsc-debug.md). 
* .vscode/settings.json - Place your settings in this file to overwrite default and user settings, by default specifies the typescript server. If you are receiving a *TypeScript tsserver error* you can install this into your project's local node_modules by exectuing `npm i typescript`.
* .gitignore - Tells Git version control which patterns to ignore.
* README.md - README file describing your application.
* electron-dotnet-quickstart.md - A Quick Start guide for you.
* .eslintrc.json - Configuring ESLint

## Running the application

To run the application we will need to install 'electron-dotnet' module which provides all of the ```Node.js``` implemenation for running within ```Electron```.

> :bulb: This project dependency will be automatic in the future once the project workflow has been defined and will be installed with the template in the future.

But right now we will have to do this install manually from the command line.

``` bash
# Windows
scriptingelectron> npm install path-to-WebSharp\electron-dotnet   
scriptingelectron> npm start 
```

``` bash
# Mac OSX
scriptingelectron$ npm install path-to-WebSharp/electron-dotnet   
scriptingelectron$ npm start 
```

## Welcome

You should be presented with the following screen:

![ScriptingElectron application](./screenshots/scriptingelectron.png)

The ScriptingElectron program writes output to the Console.  To see this you can select `View` -> `Toggle Developer Tools` which will show the console of the Developer Tools.

![ScriptingElectron application console](./screenshots/scriptingelectron-console.png)

## Scripting Electron

What the actual [scripting.js](#electron-dotnetjs-net-and-nodejs-in-process-implementation-scriptingelectronjs) code does is call a C# function to format a string using the string `Electron` as a parameter to call the function.  Once the C# function formats the string it will be returned back to the calling JavaScript function and printed out using the JavaScript function `console.log`.  What we will be doing in the following exercise is instead of returning the formatted string back to the JavaScript code to be written to the console is to write that string directly from the C# program itself by scripting the `console.log` function itself and using it from the C# program.

### Writing the library

From a command line you will creating a new `.cs` source file and `project.json` file.

``` bash
.
|--- src                              // sources
     |--- scriptingelectron.cs              // C# implementation 
     |--- project.json                // Defines compilation information 
     |--- scriptingelectron.js              // javascript code implementation
```

Using the [.NET Core.](https://www.microsoft.com/net/core) SDK create a new library source project.

``` bash
# Windows
cd src
\src> dotnet new -t Lib
\src> rename Library.cs scriptingelectron.cs
```

``` bash
# Mac OSX
\src$ dotnet new -t Lib
\src$ mv Library.cs scriptingelectron.cs
```

> :bulb: Notice the use of `-t Lib` as the parameter to the `dotnet new` command.  This specifies to the `Net Core` CLI command to create a 'library' template.  More information about the 'dotnet new' and its options can be found [here](https://github.com/dotnet/docs/blob/master/docs/core/tools/dotnet-new.md#options). 

Afterwards, you should have the same structure mentioned above in the `src` directory.

Before we can compile our code we will need to make a couple of modifications to the `package.json` file.

Open the `package.json` file in Visual Studio Code and make sure it looks as follows:

- Add a line to the `"buildoptions"` to set the `outputName` of the generated library to `scriptingelectron`.  This will be the name of our `.dll` that will be referenced from our `ScriptingElectron.js` file.

    ``` json
    "buildOptions": {
        "debugType": "portable",
        "outputName": "scriptingelectron"
    },
    ```

- Add a dependency to `WebSharp.js` package that will allow the C# program to script `NodeJs` functions.

    ``` json
    "dependencies": {
        "WebSharp.js" : { "version": "*"}
    },
    ```

- Create a ```runtimes``` section in your `project.json` file that defines the platforms your app targets, and specify the runtime identifier of each platform that you target. See Runtime IDentifier catalog for a list of runtime identifiers. For example, the following runtimes section indicates that the app runs on `64-bit Windows 10` operating systems and the `64-bit OS X Version 10.12` operating system.

    ``` json
    "runtimes": {
        "win10-x64": {},
        "osx.10.12-x64": {}
    },
    ```

> :bulb: Depending on which runtime you are targeting, you may have some `Unable to resolve` warnings output in the compiling section.

More information about the [.NET Standard](https://blogs.msdn.microsoft.com/dotnet/2016/09/26/introducing-net-standard/)

For you convenience the full `package.json` file can be copied from below.

``` json
{
  "version": "1.0.0-*",
  "buildOptions": {
    "debugType": "portable",
    "outputName": "scriptingelectron"
  },
  "runtimes": {
    "win10-x64": {},
    "osx.10.12-x64": {}
  },  
  "dependencies": {
      "WebSharp.js" : { "version": "*"}
  },
  "frameworks": {
    "netstandard1.6": {
      "dependencies": {
        "NETStandard.Library": "1.6.0"
      }
    }
  }
}
```

### Writing the implementation code

We now need to write the implementation code of our `scriptingelectron.cs`.

Open the file and replace the existing implemenation text with the following:

``` csharp

using System;
using System.Threading.Tasks;

// Reference WebSharpJs
using WebSharpJs;

namespace ScriptingElectron
{
    public class ContextMenu
    {
        public async Task<object> AddContextMenu(object input)
        {
            Func<object, Task<object>> consoleLog = await WebSharp.CreateJavaScriptFunction(@"
                                return function (data, callback) {
                                    console.log(data);
                                    callback(null, null);
                                }
                            ");

           Func<object, Task<object>> contextMenu = await WebSharp.CreateJavaScriptFunction(@"
                                return function (contextMenuCallBacks, callback) {
                                    const {remote} = require('electron')
                                    const {Menu, MenuItem} = remote

                                    const menu = new Menu()

                                    menu.append(new MenuItem({label: 'MenuItem1', click() { contextMenuCallBacks.item1click(null, null); }}))
                                    menu.append(new MenuItem({type: 'separator'}))
                                    menu.append(new MenuItem({label: 'MenuItem2', type: 'checkbox', checked: true, click() { contextMenuCallBacks.item2click(null, null); }}))

                                    window.addEventListener('contextmenu', (e) => {
                                        e.preventDefault()
                                        menu.popup(remote.getCurrentWindow())
                                    }, false)                                                     
                                    
                                    callback(null, null);
                                }
                            ");
            try
            {

                var contextMenuCallBacks = new
                {
                    item1click = (Func<object, Task<object>>)(async (commands) =>
                    {
                        consoleLog($"C# callback: Item 1 Clicked");
                        return null;
                    }),
                    item2click= (Func<object, Task<object>>)(async (commands) =>
                    {
                        consoleLog($"C# callback: Item 2 Clicked");
                        return null;
                    })
                };

                contextMenu(contextMenuCallBacks);
            }
            catch (Exception exc) { consoleLog($"Exception: {exc.Message}"); }

            return null;

        }
    }
}





```

Let's now take a look at the code.

- Using section

    Add a reference to the `WebSharpJs` assembly.

    ``` csharp
    using System;
    using System.Threading.Tasks;

    // Reference WebSharpJs 
    using WebSharpJs;
    ```

    The `WebSharpJs` assembly provides the interaction with `Nodejs`.  This managed assembly exposes the static function `CreateJavaScriptFunction` which will be detailed more below.

- Class `ContextMenu` implementation

    ``` csharp
    namespace ScriptingElectron
    {
        public class ContextMenu
        {
            public async Task<object> AddContextMenu(object input)
            {
                // Implementation here
                return null;
            }
        }

    }

    ```

    The `AddContextMenu` method follows the `Func<object,Task<object>>` delegate signature.  This is the method that will be referenced from the `scriptingelectron.js` file implementation.

- Implementing the `consoleLog` function.

    ``` csharp
           Func<object, Task<object>> consoleLog = await WebSharp.CreateJavaScriptFunction(@"
                                return function (data, callback) {
                                    console.log(data);
                                    callback(null, null);
                                }
                            ");
    ```

    The workhorse in the above code is the static function `WebSharp.CreateJavaScriptFunction` that the `WebSharp.js` managed assembly makes available to developers.  
    
    The `CreateJavaScriptFunction` accepts a string containing code in `Nodejs`, compiles it and returns a `JavaScript` function callable from the C# implementation.  The `JavaScript` function must have the following signature:

    * It must accept one parameter and a callback.  `return function (data, callback)`
    * The callback must be called with an error and one return value.  `callback(null, null);`

    The following is the pattern when scripting `Nodejs` functions:

``` javascript
        return function (data, callback) {
        
            //...... Your implementation code here .......

            callback(null, null);
        }

```

- Implementing the `contextMenu` scripting function.

``` csharp
            Func<object, Task<object>> contextMenu = await WebSharp.CreateJavaScriptFunction(@"
                                return function (contextMenuCallBacks, callback) {
                                    const {remote} = require('electron')
                                    const {Menu, MenuItem} = remote

                                    const menu = new Menu()

                                    menu.append(new MenuItem({label: 'MenuItem1', click() { contextMenuCallBacks.item1click(null, null); }}))
                                    menu.append(new MenuItem({type: 'separator'}))
                                    menu.append(new MenuItem({label: 'MenuItem2', type: 'checkbox', checked: true, click() { contextMenuCallBacks.item2click(null, null); }}))

                                    window.addEventListener('contextmenu', (e) => {
                                        e.preventDefault()
                                        menu.popup(remote.getCurrentWindow())
                                    }, false)                                                     
                                    
                                    callback(null, null);
                                }
                            ");
```

The code above can be viewed from the `Electron` documentation of the [Menu Class](https://github.com/electron/electron/blob/master/docs/api/menu.md#menubuildfromtemplatetemplate).

When creating the function we follow the same pattern as the `consoleLog`


``` javascript
        return function (contextMenuCallBacks, callback) {
        
            //...... Your implementation code here .......

            callback(null, null);
        }

```

* Require electron 
    ``` js
    const {remote} = require('electron')
    ```

* Reference the `Menu` and `MenuItem` classes

    ``` js
    const {Menu, MenuItem} = remote
    ```

* Create a new Menu class

    ``` js
    const menu = new Menu()
    ```    

* Append new menu items

    ``` js
        menu.append(new MenuItem({label: 'MenuItem1', click() { contextMenuCallBacks.item1click(null, null); }}))
        menu.append(new MenuItem({type: 'separator'}))
        menu.append(new MenuItem({label: 'MenuItem2', type: 'checkbox', checked: true, click() { contextMenuCallBacks.item2click(null, null); }}))
    ```    

    - The menu `click()` functions will call back into the C# code using callback functions passed through the `contextMenuCallBacks` parameter.

        -  menu.append(new MenuItem({label: 'MenuItem1', click() { contextMenuCallBacks.item1click(null, null); }}))
        -  menu.append(new MenuItem({label: 'MenuItem2', type: 'checkbox', checked: true, click() { contextMenuCallBacks.item2click(null, null); }}))

* Add an event listener to the `window` that will show the popup context menu.

    ``` js
    window.addEventListener('contextmenu', (e) => {
        e.preventDefault()
        menu.popup(remote.getCurrentWindow())
    }, false)           
    ```

* Define a `contextMenuCallBacks` object to be passed as a parameter to the scripted `contextMenu` function. 

``` csharp
            var contextMenuCallBacks = new
            {
                item1click = (Func<object, Task<object>>)(async (commands) =>
                {
                    consoleLog($"C# callback: Item 1 Clicked");
                    return null;
                }),
                item2click= (Func<object, Task<object>>)(async (commands) =>
                {
                    consoleLog($"C# callback: Item 2 Clicked");
                    return null;
                })
            };
```

- The `item1click` and `item2click` function delegates are referened in the scripted menu items.

    -  menu.append(new MenuItem({label: 'MenuItem1', click() { `contextMenuCallBacks.item1click`(null, null); }}))
    -  menu.append(new MenuItem({label: 'MenuItem2', type: 'checkbox', checked: true, click() { `contextMenuCallBacks.item2click`(null, null); }}))

- Call the `contextMenu` function passing the `contextMenuCallBacks` object as a parameter.

``` csharp
        contextMenu(contextMenuCallBacks);
```

### Compiling `scriptingelectron.cs` code
Before running our application we will need to compile our library code ```scriptingelectron.cs```.

The compile target uses [Dotnet Core](https://www.microsoft.com/net/core) with one of the dependencies for the `WebSharp.js` delivered as a NuGet package.

``` bash
# Windows
scriptingelectron> cd src
scriptingelectron\src> dotnet restore -s path-to-WebSharp\electron-dotnet\tools\build\nuget
scriptingelectron\src> dotnet build
scriptingelectron\src> dotnet publish
cd ..
```

``` bash
# Mac OSX
scriptingelectron$ cd src
scriptingelectron\src$ dotnet restore -s path-to-WebSharp/electron-dotnet/tools/build/nuget
scriptingelectron\src$ dotnet build
scriptingelectron\src$ dotnet publish
cd ..
```

#### What do the commands above do?

* Resolve the build assets by typing `dotnet restore`.
  * Running `restore` pulls down the required packages declared in the project.json file.
  * You'll see a new project.lock.json file in your project folder.
  * This file contains information about your project's dependencies to make subsequent restores quicker.
  * The `-s path-to-WebSharp/electron-dotnet/tools/build/nuget` in the `restore` is the nuget source where the `WebSharp.js.xxx.nupkg` can be found.
     * On Windows if a Local Package source is setup then the source will be search so you will not need to provide this parameter.
     * On Mac it seems that the `restore` does not work for Local Package sources right now.  Your mileage may vary but this is the surefire way to get the dependencies restored correctly.
     
     > :bulb: See information about [Package Sources](./vsc-package-sources.md) for more information.
     
  * The `WebSharp.js.xxxx.nupkg` dependency is built during the Electron DotNet build process.
* Build the source `scriptingelectron.cs` implementation by typing `dotnet build`.
  * The `build` command will compile the source file based on the definition found in the `project.json`
* Make the assemblies available for use by typing `dotnet publish`.
  * This will copy the implementation as well as the `WebSharp.js.dll` from the nuget package available to be loaded.

> :bulb: For more information on the publish command see the [dotnet publish documentation](https://docs.microsoft.com/en-us/dotnet/articles/core/tools/dotnet-publish)

#### DotNet Publish output folder

``` bash
.
|--- src                                             // sources
     |--- bin                                        // 
         |--- Debug
               |--- netstandard1.6            
                    |--- (runtime)                   // This folder contains the [framework]/[runtime] output as per Dot Net docs.
                    |--- scriptingelectron.dll
     |--- obj                                        // object folder
     |--- scriptingelectron.cs                             // C# implementation
     |--- scriptingelectron.js                             // javascript code implementation
     |--- project.json                               // Defines compilation information 

```

### Bringing it all together

We should now have our library implementation complete and all that is left to do is execute it to bring this all together.

Open the `scriptingelectron.js` file that was automatically generated from the template when creating the project.

``` js
var dotnet = require('electron-dotnet');

var hello = dotnet.func('async (input) => { return ".NET welcomes " + input.ToString(); }');

//Make method externaly visible this will be referenced in the renderer.js file
exports.sayHello = arg => {
	hello('Electron', function (error, result) {
		if (error) throw error;
		console.log(result);
	});
}
```

What we need to do is call into our `scriptingelectron` library code that was created in the previous steps.

So let us replace the `var hello = dotnet.func('async (input) => { return ".NET welcomes " + input.ToString(); }');`.


Mac OSX can reference the `scripting.dll` directly that was built using the the `dotnet` CLI in the previous step or using [framework]/[runtime] directory.

The two ways to reference the assemblyFile.

* Reference the `scriptingelectron.dll` directly from the framework directory 

``` js

    var addContextMenu = dotnet.func({
        assemblyFile: __dirname + '/bin/Debug/netstandard1.6/scriptingelectron.dll',
        typeName: 'ScriptingElectron.ContextMenu',
        methodName: 'AddContextMenu' // This must be Func<object,Task<object>>
    });
```

* Reference the `scriptingelectron.dll` directly from the [framework]/[runtime] directory 

``` js
    var addContextMenu = dotnet.func({
        assemblyFile: __dirname + '/bin/Debug/netstandard1.6/osx.10.12-x64/publish/scriptingelectron.dll',
        typeName: 'ScriptingElectron.ContextMenu',
        methodName: 'AddContextMenu' // This must be Func<object,Task<object>>
    });
```

Either way works for OSX but Windows can only be run from the [framework]/[runtime] directory.  So for Windows we will need to use the following:

``` js
    var addContextMenu = dotnet.func({
        assemblyFile: __dirname + '/bin/Debug/netstandard1.6/win10-x64/publish/scriptingelectron.dll',
        typeName: 'ScriptingElectron.ContextMenu',
        methodName: 'AddContextMenu' // This must be Func<object,Task<object>>
    });
```

> :bulb: Windows will throw something like the following `"Uncaught System.IO.FileNotFoundException: Could not load file or assembly 'System.Runtime ...' or one of its dependencies.` if not called with the `[framework]/[runtime]` path.

* Change our exported variable from `sayHello` to `addContextMenu`

``` js
    //Make method externaly visible this will be referenced in the renderer.js file
    exports.addContextMenu = arg => {
        addContextMenu("", function (error, result) {
            if (error) throw error;
            console.log(result);
        });
    } 
```

* We will also have to change our call from the `renderer.js` definition.

``` js
    // This file is required by the index.html file and will
    // be executed in the renderer process for that window.
    // All of the Node.js APIs are available in this process.
    var contextMenu = require("./src/scriptingelectron.js");

    contextMenu.addContextMenu();

```

#### scriptingelectronjs full source

The full code for `scriptingelectron.js` is available here for your copying convenience.

``` js
var dotnet = require('electron-dotnet');

var addContextMenu = dotnet.func({
    assemblyFile: __dirname + '/bin/Debug/netstandard1.6/scriptingelectron.dll',
    typeName: 'ScriptingElectron.ContextMenu',
    methodName: 'AddContextMenu' // This must be Func<object,Task<object>>
});

//Make method externaly visible this will be referenced in the renderer.js file
exports.addContextMenu = arg => {
	addContextMenu("", function (error, result) {
		if (error) throw error;
		if (result) console.log(result);
	});
}
```

## Running the application

To run the application we will need to install `electron-dotnet` module which provides all of the ```Node.js``` implemenation for running within ```Electron```.

> :bulb: This project dependency will be automatic in the future once the project workflow has been defined and will be installed with the template in the future.

But right now we will have to do this install manually from the command line.

``` bash
# Windows
scriptingelectron\src> cd ..
scriptingelectron> npm install path-to-WebSharp\electron-dotnet   
scriptingelectron> npm start 
```

``` bash
# Mac OSX
scriptingelectron/src$ cd ..
scriptingelectron$ npm install path-to-WebSharp/electron-dotnet   
scriptingelectron$ npm start 
```

## Scripting Electron from CLR Context Menu

You should be presented with the same screen as when we first started the application above:

![ScriptingJS application](./screenshots/scriptingelectron.png)

To see the console when the menu items are selected open the `Developer Tools` by selecting `View` -> `Toggle Developer Tools` which will show the console of the Developer Tools.

![ScriptingJS application console log](./screenshots/scriptingelectron-console2.png)

Right click on the application menu to show the context menu.

![ScriptingJS application console log](./screenshots/scriptingelectron-console3.png)

Selecting any of the menu items from the context menu will execute the corresponding callback function into the C# code.

![ScriptingJS application console log](./screenshots/scriptingelectron-menuitem-clicked.png)


## Debugging

You can now move on to [Debugging](./vsc-debug.md) your `Electron` application.