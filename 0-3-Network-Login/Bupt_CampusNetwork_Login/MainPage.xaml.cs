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

		public MainPage()
		{
			this.InitializeComponent();
			model = new AppModel();

			if (model.id != null)
				uid.Text = model.id;
			if (model.password != null)
				password.Password = model.password;
		}

		protected override async void OnNavigatedTo(NavigationEventArgs e)
		{
			base.OnNavigatedTo(e);

			try
			{
				var url = await Module.GetUrl ();
				if (url != "")
					await Module.Login(url, model.id, model.password);
				else
					Module.Toast("", "Well Done!", "Connected already :-)");
			}
			catch (Exception)
			{
				Module.Toast("", "Oh!", "No Network Connection...");
			}
		}

		private async void Button_Click(object sender, RoutedEventArgs e)
		{
			model.id = uid.Text;
			model.password = password.Password;

			try
			{
				var url = await Module.GetUrl();
				if (url != "")
					await Module.Login(url, model.id, model.password);
				// else Connected already
			}
			catch (Exception)
			{
				Module.Toast("", "Oh!", "No Network Connection...");
			}
		}
	}
}
