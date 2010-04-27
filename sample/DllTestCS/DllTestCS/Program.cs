using System;
using System.Linq;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Diagnostics;
 
namespace DllTestCS
{
    static class Program
    {
        /// <summary>
        /// アプリケーションのメイン エントリ ポイントです。
        /// </summary>
        [MTAThread]
        static void Main2()
        {
                using (Form1 fm = new Form1())
                {
                    fm.ShowDialog();
                }
        }
        //5秒キャプチャ→終了を繰り返す。(再起動実験)
        static void Main()
        {
            for (int i = 1;i<=1000 ; i++)
            {
                using (Form1 fm = new Form1())
                {
                    fm.Show();
                    DateTime d;
                    DateTime d2;
                    d = d2=DateTime.Now;
                    while (d2.Ticks - d.Ticks < 50000000)
                    {
                        Application.DoEvents();
                        d2 = DateTime.Now;
                    }
                }
                Debug.WriteLine("#"+i);
            }
        }
    }
}