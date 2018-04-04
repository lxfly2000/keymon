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

namespace keymonp
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            Focus();//http://stackoverflow.com/questions/33682783/wpf-menuitem-grayed-out-when-binding-command
        }

        KeyboardHook kh;

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            kh.UnHook();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            kh = new KeyboardHook();
            kh.SetHook();
            kh.KeyPressFunction = new KeyboardHook.KeyPressProc(kh_OnKeyPress);
            zoom_multiplier = 1.0f;
            ResizeWindow();
        }

        void kh_OnKeyPress(KeyboardHook.KeyEnum k, bool pressed)
        {
            Label label = labelShift;
            Rectangle rect = rectShift;
            switch (k)
            {
                case KeyboardHook.KeyEnum.kShift: label = labelShift; rect = rectShift; break;
                case KeyboardHook.KeyEnum.kZ:label = labelZ;rect = rectZ;break;
                case KeyboardHook.KeyEnum.kX:label = labelX;rect = rectX;break;
                case KeyboardHook.KeyEnum.kC:label = labelC;rect = rectC;break;
                case KeyboardHook.KeyEnum.kLeft:label = labelLeft;rect = rectLeft;break;
                case KeyboardHook.KeyEnum.kUp:label = labelUp;rect = rectUp;break;
                case KeyboardHook.KeyEnum.kDown:label = labelDown;rect = rectDown;break;
                case KeyboardHook.KeyEnum.kRight:label = labelRight;rect = rectRight;break;
            }
            label.Foreground = (pressed ? Brushes.Black : Brushes.White);
            rect.Fill = (pressed ? Brushes.White : Brushes.Black);
        }

        private void ShowTips_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            MessageBox.Show("本程序可以监视键盘的按下状态并实时显示。\n\nBy lxfly2000",
                "按键显示程序", MessageBoxButton.OK);
        }

        private void Close_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            Close();
        }

        private void Minimize_Executed(object sender,ExecutedRoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        private void DragWindow(object sender, MouseButtonEventArgs e)
        {
            DragMove();
        }

        private String zoom_format = "";
        private float zoom_multiplier;

        private void ResizeWindow()
        {
            MenuItem zoomItem = (MenuItem)FindName("MenuItem_Zoom");
            Viewbox viewbox = (Viewbox)FindName("ViewBox_Main");
            Grid grid = (Grid)FindName("Grid_Main");
            viewbox.Width = grid.Width * zoom_multiplier;
            viewbox.Height = grid.Height * zoom_multiplier;
            if (zoom_format.Equals("")) zoom_format = (String)zoomItem.Header;
            String zoomString = String.Format(zoom_format, (int)(zoom_multiplier * 100.0f));
            zoomItem.Header = zoomString;
        }

        private void ZoomIn_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            zoom_multiplier = Math.Min(8.0f, zoom_multiplier + 0.25f);
            ResizeWindow();
        }

        private void ZoomOut_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            zoom_multiplier = Math.Max(0.25f, zoom_multiplier - 0.25f);
            ResizeWindow();
        }
    }
}
