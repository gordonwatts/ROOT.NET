#
# simple histo fill test under python
#
import ROOT

t = ROOT.TStopwatch()
t.Start()
h = ROOT.TH1F("hi", "there", 10, 0.0, 100.0)
for i in range(1000000):
	h.Fill(4)
t.Stop()
h.Print()
t.Print()
