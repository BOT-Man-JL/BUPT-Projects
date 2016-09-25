# Bupt Campus Network Login

A UWP App to help **Bupt** students **login** their **Campus Network**
without entering Pass every time.

## Motivation

- It's a disaster to login **manually** every time
- It's very **Unsafe** to **Save Password in Browser**
- The BUPT Portal Web Site saved **PLAIN TEXT PASSWORD** in your **Browser's Cookie** !!! :scream:

``` Javascript
	var thisform=document.forms[0];
	if(thisform.DDDDD.value=="") {
		thisform.DDDDD.focus();
		alert("请您输入用户名");
		return false;
	} else if(thisform.upass.value=="") {
		thisform.upass.focus();
		alert("请您输入密码");
		return false;
	} else {
		setCookie("myusername",thisform.DDDDD.value,10);
		if(thisform.savePWD.checked)
		  setCookie("pwd",thisform.upass.value,10);
		else
		  setCookie("pwd","",-10);
		setCookie("username",thisform.DDDDD.value,10)
		setCookie("smartdot",thisform.upass.value,10)
	}
```

## Features

- **Auto** Login
- **Configurable** for Different Network ID
- **Toast Notification** Support
- Support **Desktop** and **Mobile**

## Usage

- Enable *Developer Mode* in *[Setting/Updates and Security](ms-settings:developers)*
- ~~Download the Latest [Release](https://github.com/BOT-Man-JL/Bupt_CampusNetwork_Login/releases), in *zip*~~
- **Compile** the Project and **Publish** with the key in *zip*
- Desktop
  - ~~Unsip~~
  - Right-Click the *Add-AppDevPackage.ps1* file and Open with *PowerShell*
- Mobile
  - ~~Unzip and~~ Copy the *appxbundle* file to Your Mobile
  - Open *appxbundle* on Your Mobile
- And Enjoy it :smile:
