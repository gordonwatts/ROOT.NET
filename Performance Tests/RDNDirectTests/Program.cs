using ROOTNET;

namespace RDNDirectTests
{
    class Program
    {
        static void Main(string[] args)
        {
            var t = new NTStopwatch();
            t.Start();
            var h = new NTH1F("hi", "there", 10, 0.0, 100.0);
            for (int i = 0; i < 100000000; i++)
            {
                h.Fill(10);
            }
            t.Stop();
            h.Print();
            t.Print();
        }
    }
}