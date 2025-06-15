using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace IPcalcWPF {
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window {
		public uint iIPaddress {  set; get; }
		public MainWindow() {
			InitializeComponent();
		}

		private void IPaddressChanged(object sender, RoutedEventArgs e) {
			if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 224) nud_PrefixPicker.Value = 24;
			else if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 192) nud_PrefixPicker.Value = 16;
			else if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 128) nud_PrefixPicker.Value = 8;
			else if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 1) nud_PrefixPicker.Value = 8;
		}

		private void nud_PrefixPicker_ValueChanged(object sender, RoutedPropertyChangedEventArgs<object> e) {
			if(nud_PrefixPicker.Value >= 8) {
				ipf_IPmask.IpFirstByte = "255";
			}
			else if (nud_PrefixPicker.Value >= 16) {
				ipf_IPmask.IpFirstByte = "255";
				ipf_IPmask.IpSecondByte = "255";
			}
			else if (nud_PrefixPicker.Value >= 24) {
				ipf_IPmask.IpFirstByte = "255";
				ipf_IPmask.IpSecondByte = "255";
				ipf_IPmask.IpThirdByte = "255";
			}
		}

		private void nud_PrefixPicker_Spinned(object sender, Xceed.Wpf.Toolkit.SpinEventArgs e) {
			
		}

		private void IPaddressChanged(object sender, DependencyPropertyChangedEventArgs e) {
			if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 224) nud_PrefixPicker.Value = 24;
			else if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 192) nud_PrefixPicker.Value = 16;
			else if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 128) nud_PrefixPicker.Value = 8;
			else if (Convert.ToInt32(ipf_IPaddress.IpFirstByte) > 1) nud_PrefixPicker.Value = 8;

			//ipf_IPmask.IpAddressBytes = Convert.ToUInt32(~(0xFFFFFFFF >> nud_PrefixPicker.Value));
		}
	}
}
