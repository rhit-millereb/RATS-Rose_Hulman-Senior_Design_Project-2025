using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WpfControlLibrary
{
    [WinTak.Framework.Tools.Attributes.Button("Super_Simple_Plugin_Button", "Plugin2",
LargeImage = "pack://application:,,,/WinTAK_Simple_Usage_Plugin;component/Img/tech.png",
SmallImage = "pack://application:,,,/WinTAK_Simple_Usage_Plugin;component/Img/tech.ico")]

    internal class ButtonClass : WinTak.Framework.Tools.Button
    {
        private WinTak.Framework.Docking.IDockingManager _dockingManager;

        [System.ComponentModel.Composition.ImportingConstructor]
        public ButtonClass(WinTak.Framework.Docking.IDockingManager dockingManager)
        {
            _dockingManager = dockingManager;
        }

        protected override void OnClick()
        {
            base.OnClick();

            var pane = _dockingManager.GetDockPane(DockPaneClass.ID);
            pane?.Activate();
        }
    }
}

