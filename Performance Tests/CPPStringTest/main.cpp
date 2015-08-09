//
// Test string setting
//

#include <TH1F.h>
#include <TStopwatch.h>
#include <string>

void main()
{
	auto t = new TStopwatch();

	auto h = new TH1F ("hi", "there", 100, 0.0, 10.0);
	auto a = h->GetXaxis();
	std::string str ("fork");
	t->Start();
	for (int i = 0; i < 20000000; i++) {
		a->SetBinLabel(1, str.c_str());
	}
	t->Stop();
	t->Print();
}
