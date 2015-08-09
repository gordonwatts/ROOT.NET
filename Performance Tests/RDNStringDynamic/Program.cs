using ROOTNET.Utility;
using System;

namespace RDNStringDynamic
{
    class Program
    {
        static void Main(string[] args)
        {
            var t = ((dynamic)ROOTCreator.ROOT).TStopwatch();
            var h = ((dynamic)ROOTCreator.ROOT).TH1F("hi", "there", 10, 0.0, 100.0);
            var a = h.GetXaxis();

            string newname = "fork";
            t.Start();
            for (int i = 0; i < 9000; i++)
            {
                a.SetBinLabel(1, newname);
            }
            t.Stop();
            t.Print();
        }
    }
}
