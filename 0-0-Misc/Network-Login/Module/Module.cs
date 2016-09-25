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

		private bool _debugMode;
		public bool debugMode
		{
			get { Load(ref _debugMode, true); return _debugMode; }
			set { Save(ref _debugMode, value, true); }
		}

		private bool _promptScan;
		public bool promptScan
		{
			get { Load(ref _promptScan, true); return _promptScan; }
			set { Save(ref _promptScan, value, true); }
		}

		private string[] _configs;
		public string[] configs
		{
			get { Load(ref _configs, true); return _configs; }
			set { Save(ref _configs, value, true); }
		}
	}

	public static class Module
	{
		public static async Task<string> Peek(string url)
		{
			using (var client = new HttpClient())
			{
				try
				{
					var result = await client.GetAsync(url);
					result.EnsureSuccessStatusCode();

					return await GetTitle(result.Content);
				}
				catch (Exception e)
				{
					return e.Message;
				}
			}
		}

		public static async Task<string> Login(string url, string uid, string password)
		{
			using (var client = new HttpClient())
			{
				//client.DefaultRequestHeaders.UserAgent.ParseAdd
				//	("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36" +
				//	" (KHTML, like Gecko) Chrome/46.0.2486.0 Safari/537.36 Edge/13.10586");

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

					return await GetTitle(result.Content);
				}
				catch (Exception e)
				{
					return e.Message;
				}
			}
		}

		private static async Task<string> GetTitle(HttpContent content)
		{
			var htmlPage = await content.ReadAsStringAsync();
			var charset = Regex.Match(htmlPage, "charset=(.*?)\">").Groups[1].Value;

			using (var stream = await content.ReadAsStreamAsync())
			{
				Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
				var reader = new StreamReader(stream, Encoding.GetEncoding(charset));
				htmlPage = reader.ReadToEnd();
			}

			return Regex.Match(htmlPage, "<title>(.*?)</title>").Groups[1].Value;
		}

		public static string GetUrlFromCurrentConnection()
		{
			var url = "";
			var connection = NetworkInformation.GetInternetConnectionProfile();
			if (connection != null && connection.IsWlanConnectionProfile)
			{
				var model = new AppModel();
				foreach (var config in model.configs)
				{
					var segs = config.Split(',');
					if (segs.Count() <= 1)
						continue;

					foreach (var ssid in segs)
						if (ssid == connection.ProfileName)
						{
							url = segs[0];
							break;
						}

					if (url != "")
						break;
				}
			}
			return url;
		}

		public static async Task ConnectWifi()
		{
			if (await WiFiAdapter.RequestAccessAsync()
				!= WiFiAccessStatus.Allowed)
				throw new Exception("No Access to Connect Wifi");

			var adapter = (await WiFiAdapter.FindAllAdaptersAsync())[0];
			await adapter.ScanAsync();

			if (adapter.NetworkReport.AvailableNetworks.Count == 0)
				throw new Exception("No Wifi Network Connection");

			var mapping = new Dictionary<WiFiAvailableNetwork, int>();
			var model = new AppModel();
			foreach (var network in adapter.NetworkReport.AvailableNetworks)
			{
				foreach (var config in model.configs)
				{
					var segs = config.Split(',');
					if (segs.Count() <= 1)
						continue;

					foreach (var ssid in segs)
						if (ssid == network.Ssid)
						{
							mapping[network] = network.SignalBars;
							break;
						}
				}
			}

			if (mapping.Count == 0)
				throw new Exception("No Campus Wifi within the range");

			mapping.OrderByDescending(network => network.Value);
			var result = await adapter.ConnectAsync(mapping.First().Key, WiFiReconnectionKind.Automatic);

			if (result.ConnectionStatus != WiFiConnectionStatus.Success)
				throw new Exception("Failed to Connect to Campus Wifi");
		}

		public static void Toast(string url, string title, string msg)
		{
			var xmlStr = $"<toast launch=\"{url}\" activationType=";

			if (url == "Connect-Wifi")
				xmlStr += "\"foreground\">";
			else if (url.Contains("http"))
				xmlStr += "\"protocol\">";
			else
				xmlStr += "\"background\">";

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
					return (BackgroundTaskRegistration)(cur.Value);

			var builder = new BackgroundTaskBuilder();

			builder.Name = taskName;
			builder.TaskEntryPoint = taskEntryPoint;
			builder.SetTrigger(trigger);

			if (condition != null)
				builder.AddCondition(condition);

			BackgroundTaskRegistration task = builder.Register();
			task.Completed += Task_Completed;
			return task;
		}

		private static void Task_Completed(
			BackgroundTaskRegistration sender,
			BackgroundTaskCompletedEventArgs args)
		{
			var model = new AppModel();
			if (model.debugMode)
				Toast("", "Task Running", sender.Name + " is Done (for Debug)");
		}
	}
}
