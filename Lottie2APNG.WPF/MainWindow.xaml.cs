using System.Windows;
using Lottie2APNG.LibWrapper;
using System.IO;
using CommunityToolkit.Mvvm.ComponentModel;
using Microsoft.Win32;
using SkiaSharp.Skottie;
using SkiaSharp;

namespace Lottie2APNG.WPF
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    [INotifyPropertyChanged]
    public partial class MainWindow : Window
    {
        [ObservableProperty]
        private string filePath;

        [ObservableProperty]
        private bool infinitely = true;

        [ObservableProperty]
        private bool skipFirst = false;

        [ObservableProperty]
        private string loops = "3";

        [ObservableProperty]
        private string delayNum = "1";

        [ObservableProperty]
        private string delayDen = "10";

        [ObservableProperty]
        private string currentFps;

        [ObservableProperty]
        private bool useZlib = true;

        [ObservableProperty]
        private bool use7zip = false;

        [ObservableProperty]
        private bool useZopfli = false;

        [ObservableProperty]
        private string iter1 = "15";
        
        [ObservableProperty]
        private string iter2 = "15";
        
        [ObservableProperty]
        private bool palette = false;
        
        [ObservableProperty]
        private bool colorType = false;

        [ObservableProperty]
        private string imageWidth = "128";

        [ObservableProperty]
        private string imageHeight = "128";

        [ObservableProperty]
        private double progress = 0;

        [ObservableProperty]
        private string message = "";

        private Animation ani;
        private bool isBusy = false;
        public MainWindow()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        //IEnumerable<IVideoFrame> CreateFrames(int count)
        //{
        //    Random rd = new Random();
        //    for (int t = 0; t < count; t++)
        //    {
        //        Thread.Sleep(20);
        //        var map = new SkiaSharp.SKBitmap(1280, 720);
        //        int sx = rd.Next(1270);
        //        int sy = rd.Next(710);
        //        for (int i = 0; i < 10; i++)
        //            for (int j = 0; j < 10; j++)
        //                map.SetPixel(sx + i, sy + j, SkiaSharp.SKColors.Coral);
        //        yield return new BitmapVideoFrameWrapper(map); //method that generates of receives the next frame
        //    }
        //}

        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            //var videoFramesSource = new RawVideoPipeSource(CreateFrames(64))
            //{
            //    FrameRate = 30,
            //};
            //await FFMpegArguments
            //    .FromPipeInput(videoFramesSource)
            //    .OutputToFile("C:\\Users\\hikari\\Downloads\\test.apng", true, options => options
            //        .WithCustomArgument("-vcodec apng")
            //        .OverwriteExisting()
            //        .Loop(-1))
            //    .ProcessAsynchronously();
            //FileStream fs1 = new FileStream(@"C:\Users\hikari\Downloads\324-SPLIT\324-frame04.png", FileMode.Open, FileAccess.Read);
            //MemoryStream ms1 = new MemoryStream();
            //fs1.CopyTo(ms1);
            //ms1.Position = 0;
        }

        private void ButtonLoad_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            if (dialog.ShowDialog() == true)
            {
                try {
                    FileStream fs = new FileStream(dialog.FileName, FileMode.Open, FileAccess.Read);
                    using SKManagedStream fileStream = new(fs);
                    ani = Animation.Create(fileStream);
                    FilePath = dialog.FileName;

                    ImageWidth = ((int)ani.Size.Width).ToString();
                    ImageHeight = ((int)ani.Size.Height).ToString();

                    CurrentFps = $"{1}/{ani.Fps}";
                }
                catch (Exception ex) {
                    MessageBox.Show(ex.Message, "read error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                }
            }
        }

        private void ButtonExport_Click(object sender, RoutedEventArgs e)
        {
            if (ani == null) {
                MessageBox.Show("please load a lottie animation first.", "info", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }
            if (isBusy)
            {
                MessageBox.Show("please wait...", "info", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }
            Task.Run(() => {
                try
                {
                    isBusy = true;
                    Run();
                }
                catch (Exception ex)
                {
                    Progress = 0;
                    Message = "Error";
                    MessageBox.Show(ex.Message, "unexpected error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                }
                finally
                {
                    isBusy = false;
                }
            });
        }

        private void Run()
        {
            //------ Dialog ------//
            FileInfo fileInfo = new FileInfo(FilePath);
            string saveFilePath = "";
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.InitialDirectory = fileInfo.DirectoryName;
            dialog.Filter = "PNG files (*.png) | *.png";
            dialog.FileName = fileInfo.Name.Replace(fileInfo.Extension, "") + ".png";
            if (dialog.ShowDialog() == true)
            {
                saveFilePath = dialog.FileName;
            }
            else
            {
                return;
            }

            //------ Parse Data ------//
            if (!int.TryParse(ImageWidth, out var finalWidth))
            {
                MessageBox.Show("ImageWidth should be an integer", "parse error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }
            if (!int.TryParse(ImageHeight, out var finalHeight))
            {
                MessageBox.Show("ImageHeight should be an integer", "parse error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }
            if (!int.TryParse(DelayNum, out var finalDelayNum))
            {
                MessageBox.Show("DelayNum should be an integer", "parse error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }
            if (!int.TryParse(DelayDen, out var finalDelayDen))
            {
                MessageBox.Show("DelayDen should be an integer", "parse error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }
            var finalLoops = 0;
            if (!Infinitely && !int.TryParse(Loops, out finalLoops))
            {
                MessageBox.Show("Loops should be an integer", "parse error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }
            var finalIter1 = 0;
            var finalIter2 = 0;
            if (Use7zip && !int.TryParse(Iter1, out finalIter1))
            {
                MessageBox.Show("Iterations should be an integer", "parse error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }
            if (UseZopfli && !int.TryParse(Iter2, out finalIter2))
            {
                MessageBox.Show("Iterations should be an integer", "parse error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }

            //------ Prepare ------//
            Progress = 0.1;
            Message = "Preparing to export";
            ApngManager manager = new ApngManager();
            manager.SetSize(finalWidth, finalHeight);
            manager.SetFps(finalDelayNum, finalDelayDen);
            if (Infinitely)
                manager.SetLoops(0);
            else 
                manager.SetLoops(finalLoops);
            manager.SetSkipFirst(SkipFirst ? 1 : 0);
            if (UseZlib)
                manager.SetCompressionOptions(0, 0);
            else if (Use7zip)
                manager.SetCompressionOptions(1, finalIter1);
            else if (UseZopfli)
                manager.SetCompressionOptions(2, finalIter2);
            manager.SetOptimizationOptions(!Palette, !ColorType);

            SKBitmap bitmap = new SKBitmap(finalWidth, finalHeight);
            SKCanvas canvas = new SKCanvas(bitmap);

            //------ Render ------//
            Progress = 0;
            Message = "Rendering frames";
            int steps = (int)(ani.Duration.TotalSeconds / ((double)finalDelayNum / finalDelayDen));

            for (int i = 0; i <= steps; i++)
            {
                Progress = (double)(i) / steps;
                ani.Seek((double)(i) / steps);
                canvas.Clear();
                ani.Render(canvas, SKRect.Create(finalWidth, finalHeight));
                canvas.Save();
                var data = bitmap.Encode(SKEncodedImageFormat.Png, 100);
                MemoryStream ms = new MemoryStream();
                data.SaveTo(ms); 
                ms.Position = 0;
                manager.AddFrame(ms);
                ms.Close();
            }

            //------ Optimize ------//
            Progress = 0.5;
            Message = "Optimizing";
            manager.Optimize();

            //------ Save ------//
            Progress = 0;
            Message = "Compressing and saving";
            int cur = 0;
            int total = 0;
            CancellationTokenSource cts = new CancellationTokenSource();
            Task.Run(() =>
            {
                while (!cts.IsCancellationRequested)
                {
                    if (total > 0)
                    {
                        Progress = (double)cur / total;
                    }
                    Thread.Sleep(20);
                }
            });
            manager.Save(saveFilePath, ref cur, ref total);
            cts.Cancel();

            Progress = 1;
            Message = "Done";
        }
    }
}