using Bupt_CampusNetwork_Login_Module;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.ApplicationModel.Background;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Foundation.Metadata;
using Windows.Graphics.Display;
using Windows.System.Profile;
using Windows.UI;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace Bupt_CampusNetwork_Login
{
	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	sealed partial class App : Application
	{
		public App()
		{
			this.InitializeComponent();
			this.Suspending += OnSuspending;
		}

		protected override async void OnLaunched(LaunchActivatedEventArgs e)
		{
#if DEBUG
			if (System.Diagnostics.Debugger.IsAttached)
			{
				this.DebugSettings.EnableFrameRateCounter = true;
			}
#endif

			await CreateWindow();
			Module.RegBgTask();
		}

		private async Task CreateWindow()
		{
			// Set TitleBar Color and Mobile View Mode
			var view = ApplicationView.GetForCurrentView();
			if (AnalyticsInfo.VersionInfo.DeviceFamily == "Windows.Desktop")
			{
				var bgColor = Colors.LightSteelBlue;
				view.TitleBar.BackgroundColor = bgColor;
				view.TitleBar.ButtonBackgroundColor = bgColor;
			}
			else if (AnalyticsInfo.VersionInfo.DeviceFamily == "Windows.Mobile")
			{
				if (ApiInformation.IsTypePresent("Windows.UI.ViewManagement.StatusBar"))
					await StatusBar.GetForCurrentView().HideAsync();
			}

			Frame rootFrame = Window.Current.Content as Frame;
			if (rootFrame == null)
			{
				rootFrame = new Frame();
				rootFrame.NavigationFailed += OnNavigationFailed;
				Window.Current.Content = rootFrame;
			}

			rootFrame.Navigate(typeof(MainPage));
			Window.Current.Activate();
		}

		void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
		{
			throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
		}

		private void OnSuspending(object sender, SuspendingEventArgs e)
		{
			var deferral = e.SuspendingOperation.GetDeferral();
			//TODO: Save application state and stop any background activity
			deferral.Complete();
		}
	}
}
