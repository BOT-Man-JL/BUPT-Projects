using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Networking.Connectivity;
using System.Net.Http;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Text;
using Bupt_CampusNetwork_Login_Module;
using Windows.Devices.WiFi;

namespace Bupt_CampusNetwork_Login
{
	public sealed partial class MainPage : Page
	{
		AppModel model;
		List<TextBox> configBox;

		public MainPage()
		{
			this.InitializeComponent();
			model = new AppModel();

			if (model.id != null)
				uid.Text = model.id;
			if (model.password != null)
				password.Password = model.password;

			toggle.IsOn = model.debugMode;
			toggle2.IsOn = model.promptScan;

			if (model.configs == null)
			{
				model.configs = new string[]
				{
					"http://10.3.8.211,BUPT-1",
					"http://10.4.1.2,BUPT-2",
					""
				};
			}

			configBox = new List<TextBox>();
			configBox.Add(config1);
			configBox.Add(config2);
			configBox.Add(config3);

			for (var i = 0; i < 3; ++i)
				configBox[i].Text = model.configs[i];
		}

		protected override async void OnNavigatedTo(NavigationEventArgs e)
		{
			base.OnNavigatedTo(e);

			var url = Module.GetUrlFromCurrentConnection();
			if (url != "")
			{
				if (await Module.Peek(url) != "上网注销窗")
				{
					if (await Module.Login(url, model.id, model.password) == "登录成功窗")
						Module.Toast("", "Good Job!", "Login Successfully");
					else
						Module.Toast(url, "Oops!", "Some Error occured... (Click to Login Manually)");
				}
				else
					await Prompt("Status: Connected already", 5000);
			}
			else
			{
				try
				{
					await Module.ConnectWifi();
					await Prompt("Status: Connect to Campus Wifi", 5000);
				}
				catch (Exception exception)
				{
					await Prompt("Error: " + exception.Message, 5000);
				}
			}
		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			model.id = uid.Text;
			model.password = password.Password;

			var tmpArr = new string[] { "", "", "" };
			for (var i = 0; i < 3; ++i)
				tmpArr[i] = configBox[i].Text;
			model.configs = tmpArr;
		}

		private void ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
		{
			model.debugMode = toggle.IsOn;
		}

		private void ToggleSwitch2_Toggled(object sender, RoutedEventArgs e)
		{
			model.promptScan = toggle2.IsOn;
		}

		private async Task Prompt(string msg, int millisecondsDelay)
		{
			output.Visibility = Visibility.Visible;
			output.Text = msg;

			await Task.Delay(millisecondsDelay);

			output.Text = "";
			output.Visibility = Visibility.Collapsed;
		}
	}
}
