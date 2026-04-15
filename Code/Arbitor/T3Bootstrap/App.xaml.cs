using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using T3Foundation;

namespace T3
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            T3Core.Initialize();

            // Handle unhandled exceptions gracefully
            AppDomain.CurrentDomain.UnhandledException += (s, args) =>
            {
                T3Core.Log($"Unhandled exception: {args.ExceptionObject}", T3LogLevel.Critical);
            };

            DispatcherUnhandledException += (s, args) =>
            {
                T3Core.Log($"Dispatcher exception: {args.Exception.Message}", T3LogLevel.Error);
                args.Handled = true;
            };
        }
    }
}
