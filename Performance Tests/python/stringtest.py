#
# simple histo fill test under python
#
import ROOT

t = ROOT.TStopwatch()
h = ROOT.TH1F("hi", "there", 10, 0.0, 100.0)
a = h.GetXaxis()
t.Start()
for i in range(1000000):
	a.SetBinLabel(1, "fork")
t.Stop()
h.Print()
t.Print()
