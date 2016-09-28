using System.ComponentModel;
using System.Runtime.CompilerServices;
using Windows.Storage;

namespace Bupt_CampusNetwork_Login_Module
{
	//  ---------------------------------------------------------------------------------
	//  Copyright (c) Microsoft Corporation.  All rights reserved.
	// 
	//  The MIT License (MIT)
	// 
	//  Permission is hereby granted, free of charge, to any person obtaining a copy
	//  of this software and associated documentation files (the "Software"), to deal
	//  in the Software without restriction, including without limitation the rights
	//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	//  copies of the Software, and to permit persons to whom the Software is
	//  furnished to do so, subject to the following conditions:
	// 
	//  The above copyright notice and this permission notice shall be included in
	//  all copies or substantial portions of the Software.
	// 
	//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	//  THE SOFTWARE.
	//  ---------------------------------------------------------------------------------

	public abstract class BindableBase : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler PropertyChanged = delegate { };

		protected bool SetProperty<T>(ref T storage, T value,
			[CallerMemberName] string propertyName = null)
		{
			if (Equals(storage, value))
				return false;
			storage = value;
			this.OnPropertyChanged(propertyName);
			return true;
		}

		protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
		{
			this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
		}
	}


	//  ---------------------------------------------------------------------------------
	//  Copyright (c) BOT Man JL.  All rights reserved.
	// 
	//  The MIT License (MIT)
	// 
	//  Permission is hereby granted, free of charge, to any person obtaining a copy
	//  of this software and associated documentation files (the "Software"), to deal
	//  in the Software without restriction, including without limitation the rights
	//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	//  copies of the Software, and to permit persons to whom the Software is
	//  furnished to do so, subject to the following conditions:
	// 
	//  The above copyright notice and this permission notice shall be included in
	//  all copies or substantial portions of the Software.
	// 
	//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	//  THE SOFTWARE.
	//  ---------------------------------------------------------------------------------

	public abstract class SettingBase
	{
		private ApplicationDataContainer roamingSettings;
		private ApplicationDataContainer localSettings;

		public SettingBase()
		{
			roamingSettings = ApplicationData.Current.RoamingSettings;
			localSettings = ApplicationData.Current.LocalSettings;
		}

		protected bool Save<T>(ref T storage, T value, bool isLocal,
			[CallerMemberName] string propertyName = null)
		{
			if (Equals(storage, value))
				return false;
			storage = value;

			var composite = new ApplicationDataCompositeValue();
			composite["data"] = value;
			(isLocal ? localSettings : roamingSettings).Values[propertyName]
				= composite;
			return true;
		}

		protected bool Load<T>(ref T storage, bool isLocal,
			[CallerMemberName] string propertyName = null)
		{
			var composite = (ApplicationDataCompositeValue)
				(isLocal ? localSettings : roamingSettings).Values[propertyName];
			if (composite == null)
			{
				storage = default(T);
				return false;
			}

			storage = (T)composite["data"];
			return true;
		}
	}
}
