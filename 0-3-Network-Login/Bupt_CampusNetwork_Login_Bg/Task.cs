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

			if (taskInstance.TriggerDetails.GetType() ==
				typeof(ToastNotificationActionTriggerDetail))
			{
				var details = taskInstance.TriggerDetails
					as ToastNotificationActionTriggerDetail;
				if (details.Argument != "")
					await Windows.System.Launcher
						.LaunchUriAsync(new Uri(details.Argument));
			}
			else  // NetworkStateChangeEventDetails
			{
				try
				{
					var url = await Module.GetUrl();
					if (url != "")
					{
						var model = new AppModel();
						await Module.Login(url, model.id, model.password);
					}
					// else Login already
				}
				catch (Exception)
				{
					// No Connection
				}
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
