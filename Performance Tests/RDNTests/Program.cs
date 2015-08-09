using ROOTNET.Utility;
using System;
namespace RDNTests
{
    class Program
    {
        static void Main(string[] args)
        {
            var t = ((dynamic)ROOTCreator.ROOT).TStopwatch();
            t.Start();
            var h = ((dynamic)ROOTCreator.ROOT).TH1F("hi", "there", 10, 0.0, 100.0);
            for (int i = 0; i < 40000; i++)
            {
                h.Fill(10);
            }
            t.Stop();
            h.Print();
            t.Print();
        }
    }
}
