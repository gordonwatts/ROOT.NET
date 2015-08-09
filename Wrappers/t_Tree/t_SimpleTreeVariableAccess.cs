using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Tree
{
    [TestClass]
    public class t_SimpleTreeVariableAccess
    {
        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestSimpleInt()
        {
            // Open and get the tree
            var tree = Utils.OpenAndGet("btag-slim.root", "vtuple");
            foreach (dynamic evt in tree)
            {
                var r = evt.run;
                Assert.IsInstanceOfType(r, typeof(uint), "Run number type");
                Assert.AreEqual((uint) 155073, r, "Run number");
            }
        }

        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestSimpleIntLINQ()
        {
            // Open and get the tree
            var tree = Utils.OpenAndGet("btag-slim.root", "vtuple");
            Assert.IsTrue(tree.Cast<dynamic>().All(evt => evt.run == 155073), "all with same run number");
        }
    }
}
