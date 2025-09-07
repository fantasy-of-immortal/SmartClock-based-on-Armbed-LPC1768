using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace MusicPlayer
{
    public partial class MusicLibraryWindow : Window
    {
        // 假设这是您音乐库的歌曲列表
        private List<Song> songs = new List<Song>
        {
            new Song { Title = "恭喜发财                                                                         - 刘德华", FileName = "1.mp3" },
            new Song { Title = "一等情事                                                                         - 许一鸣", FileName = "2.mp3" }
        };

        public MusicLibraryWindow()
        {
            InitializeComponent();
            LoadMusicLibrary();
        }

        // 加载音乐库
        private void LoadMusicLibrary()
        {
            // 将歌曲列表绑定到ListBox
            MusicListBox.ItemsSource = songs.Select(s => s.Title).ToList();
        }

        // 双击播放指定音乐
        private void MusicListBox_MouseDoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            if (MusicListBox.SelectedItem != null)
            {
                string selectedSongTitle = MusicListBox.SelectedItem.ToString();
                var selectedSong = songs.FirstOrDefault(s => s.Title == selectedSongTitle);

                if (selectedSong != null)
                {
                    PlaySong(selectedSong.FileName);
                }
            }
        }

        // 播放音乐的功能
        private void PlaySong(string fileName)
        {
            // 这里可以调用下位机的播放功能，传入文件名
            // 例如：SerialSend($"PLAY:{fileName}");
            MessageBox.Show($"正在播放: {fileName}", "播放音乐", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        // 关闭窗口的方法
        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }

    // 歌曲类
    public class Song
    {
        public string Title { get; set; }
        public string FileName { get; set; }
    }
}
