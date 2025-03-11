using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WpfControlLibrary
{

    [Prism.Mef.Modularity.ModuleExport(typeof(ModuleClass), InitializationMode = Prism.Modularity.InitializationMode.WhenAvailable)]
    internal class ModuleClass : Prism.Modularity.IModule
    {
        private readonly Prism.Events.IEventAggregator _eventAggregator;

        [System.ComponentModel.Composition.ImportingConstructor]
        public ModuleClass(Prism.Events.IEventAggregator eventAggregator)
        {
            _eventAggregator = eventAggregator;
        }
        // Modules will be initialized during startup. Any work that needs to be done at startup can
        // be initiated from here.
        public void Initialize()
        {

        }
    }
}

