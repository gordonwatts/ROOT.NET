using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

namespace t_Dynamic
{
    [TestClass]
    public class t_NonTObjectClasses
    {
        // Force a reference to the monolithic wrapper guys - so that we can
        // make sure that the wrappers get loaded into memory.
#pragma warning disable 0414
        static ROOTNET.NTObject _obj = null;
#pragma warning restore 0414

        [TestInitialize]
        public void TestInit()
        {
            ROOTNET.Utility.ROOTCreator.ResetROOTCreator();
        }

        [TestMethod]
        public void TestTAttLineCtor()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            // This is not a known class. Make sure that we've not accidentally put this in.
            Assert.AreNotEqual("NTAxis", vec.GetType().Name, "wrapper object class");
        }

        [TestMethod]
        public void TestCallTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            vec.SetTextAngle(45.0);
            Assert.AreEqual(45.0, vec.GetTextAngle(), "Text Angle");
        }

        [TestMethod]
        public void TestGetPropertyTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            vec.SetTextAngle(50.0);
            Assert.AreEqual(50.0, vec.TextAngle, "Text Angle");
        }

        [TestMethod]
        public void TestSetPropertyTAttLine()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            vec.TextAngle = 2.0;
            Assert.AreEqual(2.0, vec.GetTextAngle, "Text Angle");
        }

        [TestMethod]
        public void TestPassByReference()
        {
            dynamic a1 = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText");
            a1.TextAngle = 2.0;
            dynamic a2 = ROOTNET.Utility.ROOTCreator.CreateByName("TAttText", a1);
            Assert.AreEqual(2.0, a2.TextAngle, "Text Angle");
        }

        [TestMethod]
        public void TestCTorWithKnownClass()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("TAttFill");
            Assert.AreEqual("NTAttFill", vec.GetType().Name, "wrapper object class");
        }

#if false
        // In order for this test to work you have to have run devenv from a cmd prompt
        // that has the location of the compiler defined!
        // Also add the following lines to monolithic to make it work:
        //asked_for_class_list.push_back ("TClass");
        //asked_for_class_list.push_back ("TSystem");
        //asked_for_class_list.push_back ("TInterpreter");

        [TestMethod]
        public void TestTParameterArgument()
        {
            var tlz = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);

            //
            // Make sure to create the TParameter guy.
            // NOTE: this program must be run in an environment that knows where cl.exe is!
            //

            var interp = ROOTNET.NTInterpreter.Instance();
            var gSystem = ROOTNET.NTSystem.gSystem;

            using (var output = File.CreateText("tlzemitter.h"))
            {
                output.WriteLine("#include \"TLorentzVector.h\"");
                output.WriteLine("ostream& operator << (ostream &os, const TLorentzVector &v) {");
                output.WriteLine(" os << \"TLZ\";");
                output.WriteLine(" return os;");
                output.WriteLine("}");
                output.WriteLine("TLorentzVector operator*= (const TLorentzVector &v1, const TLorentzVector &v2) {");
                output.WriteLine("return TLorentzVector();");
                output.WriteLine("}");
            }

            interp.GenerateDictionary("TParameter<TLorentzVector>", "tlzemitter.h;TParameter.h;TLorentzVector.h");

            //
            // Now that dict is created, can we use dynamic crap to actually go after it? :-)
            //

            var c = ROOTNET.NTClass.GetClass("TParameter<TLorentzVector>");
            Assert.IsNotNull(c, "not tclass got defined");

            dynamic param = ROOTNET.Utility.ROOTCreator.CreateByName("TParameter<TLorentzVector>", new object[] { "Dude", tlz });
            Assert.IsNotNull(param, "Unable to create TParameter");

            Assert.AreEqual("Dude", param.Name, "Name of param");
            var tlzback = param.Val;
            Assert.AreEqual(2.0, tlzback.Y, "Y value of tlz we stashed in teh parameter");
        }
#endif

    }
}
