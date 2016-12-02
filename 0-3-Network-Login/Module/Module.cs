using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Windows.ApplicationModel.Background;
using Windows.Data.Xml.Dom;
using Windows.Devices.WiFi;
using Windows.Networking.Connectivity;
using Windows.UI.Notifications;

namespace Bupt_CampusNetwork_Login_Module
{
	public class AppModel : SettingBase
	{
		private string _id;
		public string id
		{
			get { Load(ref _id, true); return _id; }
			set { Save(ref _id, value, true); }
		}

		private string _password;
		public string password
		{
			get { Load(ref _password, true); return _password; }
			set { Save(ref _password, value, true); }
		}
	}

	public static class Module
	{
		private static async Task<string> _Login(string url, string uid, string password)
		{
			using (var client = new HttpClient())
			{
				client.DefaultRequestHeaders.UserAgent.ParseAdd
					("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36" +
					" (KHTML, like Gecko) Chrome/46.0.2486.0 Safari/537.36 Edge/13.10586");

				var payload = new FormUrlEncodedContent(new List<KeyValuePair<string, string>>
				{
					new KeyValuePair<string, string> ("DDDDD", uid),
					new KeyValuePair<string, string> ("upass", password),
					new KeyValuePair<string, string> ("savePWD", "0"),
					new KeyValuePair<string, string> ("0MKKey", "")
				});

				try
				{
					var result = await client.PostAsync(url, payload);
					result.EnsureSuccessStatusCode();

					var htmlPage = await result.Content.ReadAsStringAsync();
					var charset = Regex.Match(htmlPage, "charset=(.*?)\">").Groups[1].Value;

					using (var stream = await result.Content.ReadAsStreamAsync())
					{
						Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
						var reader = new StreamReader(stream, Encoding.GetEncoding(charset));
						htmlPage = reader.ReadToEnd();
					}

					if (_GetTitle(htmlPage) == okTitle)
						return okTitle;
					else
						return "";
				}
				catch (Exception e)
				{
					return e.Message;
				}
			}
		}

		private static string _GetTitle(string htmlPage)
		{
			return Regex.Match(htmlPage, "<title>(.*?)</title>").Groups[1].Value;
		}

		public static string okTitle = "登录成功窗";

		public static async Task Login(string url, string uid, string password)
		{
			if (uid != null && password != null)
			{
				var msg = await _Login(url, uid, password);
				if (msg == okTitle)
					Toast("", "Good Job!", "Login Successfully :-)");
				else
					Toast(url, "Oops!",
						"Login Failed :-(   Click to Login in Website");
			}
			else
				Toast(null, "Welcome!",
					"Set your ID and Password first :-)");
		}

		public static async Task<string> GetUrl()
		{
			const string testUrl = "http://baidu.com";
			using (var client = new HttpClient())
			{
				var result = await client.GetAsync(testUrl);
				if (result.StatusCode == System.Net.HttpStatusCode.OK &&
					result.RequestMessage.RequestUri.Host != new Uri(testUrl).Host)
					return result.RequestMessage.RequestUri.ToString();
				return "";
			}
		}

		public static void Toast(string url, string title, string msg)
		{
			string xmlStr;
			if (url != null)
			{
				xmlStr = $"<toast launch=\"{url}\" activationType=";

				if (url.Contains("http"))
					xmlStr += "\"protocol\">";
				else
					xmlStr += "\"background\">";
			}
			else
				xmlStr = $"<toast launch=\"\" activationType=\"foreground\">";

			xmlStr +=
@"  <visual>
    <binding template=""ToastGeneric"">
      <text>" + title + @"</text>
      <text>" + msg + @"</text>
    </binding>
  </visual>
</toast>";
			var content = new XmlDocument();
			content.LoadXml(xmlStr);

			ToastNotificationManager.History.Clear();
			ToastNotificationManager.CreateToastNotifier()
				.Show(new ToastNotification(content));
		}

		public static void RegBgTask()
		{
			//var res = await BackgroundExecutionManager.RequestAccessAsync();
			//if (res == BackgroundAccessStatus.Denied || res == BackgroundAccessStatus.Unspecified)

			var taskName = "BuptNetworkHook";

			var triggers = new List<IBackgroundTrigger>();
			triggers.Add(new SystemTrigger(SystemTriggerType.NetworkStateChange, false));
			triggers.Add(new ToastNotificationActionTrigger());
			//triggers.Add(new SystemTrigger(SystemTriggerType.InternetAvailable, false));
			//triggers.Add(new SystemTrigger(SystemTriggerType.SessionConnected, false));
			//triggers.Add(new TimeTrigger(15, false));

			// Debug
			//foreach (var cur in BackgroundTaskRegistration.AllTasks)
			//	cur.Value.Unregister(true);

			int index = 0;
			foreach (var trigger in triggers)
			{
				var taskNametmp = taskName + (++index);
				var taskEntryPoint = "Bupt_CampusNetwork_Login_Bg.Task";
				var task = RegisterBackgroundTask(taskEntryPoint, taskNametmp, trigger, null);
			}
		}

		public static BackgroundTaskRegistration RegisterBackgroundTask(
			string taskEntryPoint,
			string taskName,
			IBackgroundTrigger trigger,
			IBackgroundCondition condition)
		{
			foreach (var cur in BackgroundTaskRegistration.AllTasks)
				if (cur.Value.Name == taskName)
					return (BackgroundTaskRegistration) (cur.Value);

			var builder = new BackgroundTaskBuilder();

			builder.Name = taskName;
			builder.TaskEntryPoint = taskEntryPoint;
			builder.SetTrigger(trigger);

			if (condition != null)
				builder.AddCondition(condition);

			BackgroundTaskRegistration task = builder.Register();
			//task.Completed += Task_Completed;
			return task;
		}
	}
}
