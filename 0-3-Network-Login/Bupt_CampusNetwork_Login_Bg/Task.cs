using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Windows.Data.Xml.Dom;
using Windows.ApplicationModel.Background;
using Windows.Networking.Connectivity;
using Windows.UI.Notifications;
using Bupt_CampusNetwork_Login_Module;

namespace Bupt_CampusNetwork_Login_Bg
{
	public sealed class Task : IBackgroundTask
	{
		BackgroundTaskDeferral deferral;

		public async void Run(IBackgroundTaskInstance taskInstance)
		{
			deferral = taskInstance.GetDeferral();
			taskInstance.Canceled += TaskInstance_Canceled;

			var model = new AppModel();
			if (model.debugMode)
			{
				//(taskInstance.TriggerDetails as NetworkStateChangeEventDetails).HasNewInternetConnectionProfile
				Module.Toast("", "Task Running",
					(taskInstance.TriggerDetails != null ?
					taskInstance.TriggerDetails.ToString() :
					"Something UnKnown") + " is Running (for Debug)");
			}

			if (taskInstance.TriggerDetails.GetType() == typeof(ToastNotificationActionTriggerDetail))
			{
				var details = taskInstance.TriggerDetails as ToastNotificationActionTriggerDetail;
				if (details.Argument != "")
					await Windows.System.Launcher.LaunchUriAsync(new Uri(details.Argument));
			}
			else  // NetworkStateChangeEventDetails
			{
				var url = Module.GetUrlFromCurrentConnection();
				if (url != "")
				{
					var peekTitle = await Module.Peek(url);
					if (peekTitle != "上网注销窗")
					{
						var loginTitle = await Module.Login(url, model.id, model.password);
						if (loginTitle == "登录成功窗")
							Module.Toast("", "Good Job!", "Login Successfully");
						else
							Module.Toast(url, "Oops!", "Some Error occured... (Click to Login Manually)");
					}
					// else => Connected already
				}
				else if (model.promptScan)
					Module.Toast("Connect-Wifi", "Click Me", "Click to Scan and Connect Bupt Wifi");
			}

			if (deferral != null)
				deferral.Complete();
		}

		private void TaskInstance_Canceled(
			IBackgroundTaskInstance sender,
			BackgroundTaskCancellationReason reason)
		{
			if (deferral != null)
				deferral.Complete();
		}
	}
}
