using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Tree
{
    /// <summary>
    /// Test for various root objects and our ability to read them back.
    /// </summary>
    [TestClass]
    public class t_ROOTObjects
    {
        [TestMethod]
        [DeploymentItem("tlz.root")]
        public void TestSimpleTLZ()
        {
            var t = Utils.OpenAndGet("tlz.root", "tlz");
            double count = 1.0;
            foreach (dynamic evt in t)
            {
                Assert.AreEqual(count, evt.tlz.X(), "X value");
                count += 1.0;
            }
        }
    }
}
