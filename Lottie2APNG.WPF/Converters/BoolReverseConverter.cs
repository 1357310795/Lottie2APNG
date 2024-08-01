using System;
using System.Globalization;
using System.Windows.Data;

namespace Lottie2APNG.WPF.Converters
{
    /// <summary>
    /// 布尔值翻转转换器。
    /// </summary>
    public class BoolReverseConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return !(bool)value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return !(bool)value;
        }
    }
}
