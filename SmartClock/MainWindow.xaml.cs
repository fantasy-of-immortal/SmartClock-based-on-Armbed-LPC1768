using System;
using System.IO.Ports;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Threading;
using MusicPlayer;

namespace SmartClock
{

    public partial class MainWindow : Window
    {
        private SerialPort serialPort;
        private const int BaudRate = 9600;
        private DispatcherTimer timer;

        public MainWindow()
        {
            InitializeComponent();
            InitializeSerialPort();
            LoadAvailablePorts();
            InitializeTimer();
        }

        private void InitializeSerialPort()
        {
            serialPort = new SerialPort
            {
                BaudRate = BaudRate,
                ReadTimeout = 500,
                WriteTimeout = 500
            };
            serialPort.DataReceived += SerialPort_DataReceived;
        }

        private void LoadAvailablePorts()
        {
            portComboBox.ItemsSource = SerialPort.GetPortNames();
            if (portComboBox.Items.Count > 0)
            {
                portComboBox.SelectedIndex = 0;
            }
        }

        private void InitializeTimer()
        {
            timer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(1)
            };
            timer.Tick += Timer_Tick;
            timer.Start();
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            currentTimeTextBlock.Text = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
        }

        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            if (serialPort.IsOpen)
            {
                serialPort.Close();
                MessageBox.Show("串口已断开连接", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            }
            else
            {
                if (portComboBox.SelectedItem != null)
                {
                    serialPort.PortName = portComboBox.SelectedItem.ToString();
                    try
                    {
                        serialPort.Open();
                        MessageBox.Show("串口连接成功", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show($"无法打开串口: {ex.Message}", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
                    }
                }
                else
                {
                    MessageBox.Show("请选择一个有效的串口", "错误", MessageBoxButton.OK, MessageBoxImage.Warning);
                }
            }
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                while (serialPort.BytesToRead > 0)
                {
                    string data = serialPort.ReadLine();
                    Dispatcher.Invoke(() =>
                    {
                        UpdateTemperatureHumidity(data);
                        UpdateDebugTextBox(data); // 更新调试信息
                    });
                }
            }
            catch (TimeoutException)
            {
                // 超时异常可以忽略，正常处理
            }
            catch (Exception ex)
            {
                MessageBox.Show($"数据接收错误: {ex.Message}", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void UpdateTemperatureHumidity(string data)
        {
            var match = Regex.Match(data, @"T:(\d+.\d+);H:(\d+.\d+);");
            if (match.Success)
            {
                temperatureTextBlock.Text = $"{match.Groups[1].Value} °C";
                humidityTextBlock.Text = $"{match.Groups[2].Value} %";
            }
        }

        private void UpdateDebugTextBox(string message)
        {
            DebugTextBox.AppendText(message + Environment.NewLine);
            DebugTextBox.ScrollToEnd(); // 滚动到文本框的底部
        }


            // 在主窗口中定义歌曲列表
        private List<Song> songs = new List<Song>
        {
            new Song { Title = "恭喜发财 - 刘德华", FileName = "1.mp3" },
            new Song { Title = "一等情事 - 许一鸣", FileName = "2.mp3" }
        };
        private void UpdateCurrentTrack(string data)
        {
            // 使用正则表达式从数据中提取歌曲ID
            var match = Regex.Match(data, @"Song:(\d+);");

            // 如果匹配成功，更新当前曲目的信息
            if (match.Success)
            {
                int songId = int.Parse(match.Groups[1].Value);  // 提取歌曲ID并转换为整数

                // 根据歌曲ID查找对应的歌曲
                var currentSong = songs.FirstOrDefault(s => s.FileName.StartsWith(songId.ToString()));

                // 如果找到了对应的歌曲，则更新显示
                if (currentSong != null)
                {
                    currentTrackTextBlock.Text = $"当前播放曲目：{currentSong.Title}";
                }
                else
                {
                    currentTrackTextBlock.Text = "无法找到对应的歌曲";  // 如果没有找到对应的歌曲
                }
            }
            else
            {
                currentTrackTextBlock.Text = "解析失败";  // 如果数据格式不符合预期
            }
        }

        private void AlarmTimeTextBox_GotFocus(object sender, RoutedEventArgs e)
        {
            if (alarmTimeTextBox.Text == "HH:mm")
            {
                alarmTimeTextBox.Text = string.Empty;
                alarmTimeTextBox.Foreground = System.Windows.Media.Brushes.Black;
            }
        }

        private void AlarmTimeTextBox_LostFocus(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(alarmTimeTextBox.Text))
            {
                alarmTimeTextBox.Text = "HH:mm";
                alarmTimeTextBox.Foreground = System.Windows.Media.Brushes.Gray;
            }
        }

        private void SetAlarmButton_Click(object sender, RoutedEventArgs e)
        {
            if (TimeSpan.TryParse(alarmTimeTextBox.Text, out TimeSpan alarmTime))
            {
                string repeat = repeatCheckBox.IsChecked == true ? "1" : "0";
                string command = $"SETALARM:{alarmTime.Hours:D2}:{alarmTime.Minutes:D2}:{repeat};";
                SendCommand(command);
                MessageBox.Show("闹钟设置成功", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            }
            else
            {
                MessageBox.Show("请输入有效的时间 (格式: HH:mm)", "错误", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }

        private void SyncTimeButton_Click(object sender, RoutedEventArgs e)
        {
            string currentTime = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ss");
            string command = $"SYNC:{currentTime};";

            SendCommand(command);
            MessageBox.Show("校时成功", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        private void SendCommand(string command)
        {
            if (serialPort.IsOpen)
            {
                serialPort.WriteLine(command);
            }
            else
            {
                MessageBox.Show("串口未连接，请先连接串口", "错误", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }

        private void PlayButton_Click(object sender, RoutedEventArgs e)
        {
            SendCommand("PLAY;");
        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            SendCommand("PAUSE;");
        }

        private void NextButton_Click(object sender, RoutedEventArgs e)
        {
            SendCommand("NEXT;");
        }

        private void PrevButton_Click(object sender, RoutedEventArgs e)
        {
            SendCommand("PREV;");
        }

        // 增加音量按钮点击事件
        private void VolumeUpButton_Click(object sender, RoutedEventArgs e)
        {
            // 发送音量增加命令
            SendCommand("VOL_UP;");
        }

        // 降低音量按钮点击事件
        private void VolumeDownButton_Click(object sender, RoutedEventArgs e)
        {
            // 发送音量降低命令
            SendCommand("VOL_DOWN;");
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            if (serialPort.IsOpen)
            {
                serialPort.Close();
            }
        }

        // 显示音乐库窗口
        private void ShowLibraryButton_Click(object sender, RoutedEventArgs e)
        {
            MusicLibraryWindow libraryWindow = new MusicLibraryWindow();
            libraryWindow.Show();
        }
    }
}
