using System;
using System.Collections.ObjectModel;
using System.Windows.Input;
using WinTak.Display;
using WinTak.CursorOnTarget;
using TAKEngine.Core;
using WinTak.CursorOnTarget.Graphics;
using System.ComponentModel.Composition;
using WinTak.Mapping;
using WinTak.Common.Coords;
using WinTak.Framework;
using WinTak.Framework.Docking;
using WinTak.Framework.Docking.Attributes;
using WinTak.Framework.Messaging;
using WinTak.Framework.Notifications;
using WinTak.CursorOnTarget.Services;
using MapEngine.Interop.Util;
using System.Reflection;
using WinTak.Common.CoT;
using System.Linq;
using System.Runtime.InteropServices;
using System.Timers;
using System.IO;



namespace WpfControlLibrary
{
    [WinTak.Framework.Docking.Attributes.DockPane(ID, "Super_Simple_Plugin_Example", Content = typeof(UserControl1))]
    [Export(typeof(DockPaneClass))]
    internal class DockPaneClass : WinTak.Framework.Docking.DockPane
    {
        private int _counter;
        internal const string ID = "Super_Simple_Plugin_Example_DockPane";
        internal const string TAG = "DockPane";

        //for the external console (not needed)
        [DllImport("kernel32.dll")]
        private static extern bool AllocConsole();

        private bool _isActive;
        private CoordinateViewModel _lastCoordinate;
        private CoordinateViewModel _secondLastCoordinate;
        private System.Timers.Timer _liveUpdateTimer;
        private MarkerViewModel _liveUpdateMarker;
        private bool _isLiveUpdateRunning;
        private Random _random = new Random();

        public ObservableCollection<CoordinateViewModel> CoordinatesList { get; private set; }
        public ObservableCollection<MarkerViewModel> MarkerList { get; private set; }

        public ICommand StartLiveMovementCommand { get; private set; }
        public ICommand StopLiveMovementCommand { get; private set; }

        public ICommand ToggleMarkerMovementCommand { get; private set; }
        public ICommand StartSelectedMarkersMovementCommand { get; private set; }
        public ICommand StopSelectedMarkersMovementCommand { get; private set; }
        public ICommand ToggleMovementOnNewMarkersCommand { get; private set; }

        private bool _isMovementOnByDefault = false; // if true, newly created markers start moving automatically
        public bool IsMovementOnByDefault
        {
            get => _isMovementOnByDefault;
            set => SetProperty(ref _isMovementOnByDefault, value);
        }


        [Import]
        private ICoTManager _coTManager { get; set; }

        [Import]
        private ICotMessageReceiver _cotMessageReceiver { get; set; }

        [Import]
        private IElevationManager _elevationManager { get; set; }

        // path to write csv file of lat/lon/team data and timestamps of a session to a local directory automatically - modify as needed
        private string _csvFilePath = @"C:\Users\koontzis\Documents\WinTAK_csv\file2.csv";

        // ---------------------------------------



        //CONSTRUCTOR



        //-----------------------------------------
        [ImportingConstructor]
        public DockPaneClass(ICotMessageReceiver cotMessageReceiver)
        {
            AllocConsole(); //opens external console

            Console.WriteLine("Started");

            CoordinatesList = new ObservableCollection<CoordinateViewModel>();
            MarkerList = new ObservableCollection<MarkerViewModel>();

            NotifyCommand = new ExecutedCommand(OnCommandExecuted);
            DeleteMarkerCommand = new RelayCommand(DeleteMarker);
            SetRedTeamColorCommand = new RelayCommand(SetRedTeamColor);
            SetBlueTeamColorCommand = new RelayCommand(SetBlueTeamColor);
            AddMarkerCommand = new RelayCommand(AddMarker);
            ClearCoordinatesCommand = new RelayCommand(ClearCoordinatesList);
            RemoveTeamColorCommand = new RelayCommand(RemoveTeamColor);

            // live movement commands
            StartLiveMovementCommand = new RelayCommand(StartAllMovement);
            StopLiveMovementCommand = new RelayCommand(StopAllMovement);
            ToggleMarkerMovementCommand = new RelayCommand(ToggleMarkerMovement);
            StartSelectedMarkersMovementCommand = new RelayCommand(StartSelectedMarkersMovement);
            StopSelectedMarkersMovementCommand = new RelayCommand(StopSelectedMarkersMovement);
            ToggleMovementOnNewMarkersCommand = new RelayCommand(ToggleMovementOnNewMarkers);

            // listen to map clicks and subscribes to the event of clicking on map
            WinTak.Display.MapViewControl.MapMouseDown += MapViewControl_MapMouseDown;
            _cotMessageReceiver = cotMessageReceiver;
            _cotMessageReceiver.MessageReceived += OnCotMessageReceived;

            System.Diagnostics.Debug.Listeners.Add(new System.Diagnostics.DefaultTraceListener());
            EnsureCsvHeaders();

            _liveUpdateTimer = new System.Timers.Timer(1000);
            _liveUpdateTimer.Elapsed += OnLiveUpdateTimerElapsed;
            _liveUpdateTimer.AutoReset = true;

        }

        // ---------------------------------------




        //SIMULATE MOVEMENT METHODS




        //-----------------------------------------



        private void ToggleMovementOnNewMarkers(object parameter)
        {
            IsMovementOnByDefault = !IsMovementOnByDefault;
            Console.WriteLine($"movement on newly created markers toggled: {IsMovementOnByDefault}");
        }

        private void StartAllMovement(object parameter)
        {
            if (_coTManager == null)
            {
                Console.WriteLine("It's so over.");
                return;
            }

            // get moving
            foreach (var marker in MarkerList)
            {
                marker.IsMoving = true;
            }

            StartTimerIfNeeded();
            Console.WriteLine("Markers moved");
        }

        private void StopAllMovement(object parameter)
        {
            foreach (var marker in MarkerList)
            {
                marker.IsMoving = false;
            }

            StopTimerIfNoMarkersMoving();
            Console.WriteLine("All markers movement stopped.");
        }

        private void StartSelectedMarkersMovement(object parameter)
        {
            if (parameter is MarkerViewModel marker)
            {
                marker.IsMoving = true;
            }
            else if (parameter is System.Collections.IEnumerable selectedMarkers)
            {
                foreach (var m in selectedMarkers.OfType<MarkerViewModel>())
                {
                    m.IsMoving = true;
                }
            }

            StartTimerIfNeeded();
        }

        private void StopSelectedMarkersMovement(object parameter)
        {
            if (parameter is MarkerViewModel marker)
            {
                marker.IsMoving = false;
            }
            else if (parameter is System.Collections.IEnumerable selectedMarkers)
            {
                foreach (var m in selectedMarkers.OfType<MarkerViewModel>())
                {
                    m.IsMoving = false;
                }
            }

            StopTimerIfNoMarkersMoving();
        }

        private void ToggleMarkerMovement(object parameter)
        {
            if (parameter is MarkerViewModel marker)
            {
                marker.IsMoving = !marker.IsMoving;
                if (marker.IsMoving)
                    StartTimerIfNeeded();
                else
                    StopTimerIfNoMarkersMoving();
            }
        }

        private void StartTimerIfNeeded()
        {
            if (!_isLiveUpdateRunning && MarkerList.Any(m => m.IsMoving))
            {
                _liveUpdateTimer.Start();
                _isLiveUpdateRunning = true;
            }
        }

        private void StopTimerIfNoMarkersMoving()
        {
            if (!MarkerList.Any(m => m.IsMoving))
            {
                _liveUpdateTimer.Stop();
                _isLiveUpdateRunning = false;
            }
        }

        private void OnLiveUpdateTimerElapsed(object sender, ElapsedEventArgs e)
        {
            // if the markers have ISMoving true, they move based off this random function
            foreach (var marker in MarkerList.Where(m => m.IsMoving))
            {

                double latDelta = (_random.NextDouble() - 0.5) * 0.002; // random delta between -0.001 and +0.001
                double lonDelta = (_random.NextDouble() - 0.5) * 0.002;

                marker.Lat += latDelta;
                marker.Lon += lonDelta;

                // Simulate battery drop by a random small integer percentage
                int currentBattery = ParseBatteryPercentage(marker.Battery);
                if (currentBattery > 0)
                {
                    currentBattery = Math.Max(0, currentBattery - _random.Next(0, 3));
                }
                marker.Battery = currentBattery.ToString() + "%";

                UpdateMarkerTeamColor(marker, marker.TeamColor);
                LogMarkerToCsv(marker); //make sure it logs
            }
        }

        private int ParseBatteryPercentage(string batteryStr)
        {
            if (string.IsNullOrEmpty(batteryStr)) return 100;
            if (batteryStr.EndsWith("%"))
            {
                batteryStr = batteryStr.TrimEnd('%');
            }
            if (int.TryParse(batteryStr, out int value))
            {
                return value;
            }
            return 100;
        }
        //------------------------




        // CSV LOGGING METHODS




        //------------------------

        private void EnsureCsvHeaders()
        {
            if (!File.Exists(_csvFilePath))
            {
                using (var writer = new StreamWriter(_csvFilePath, false))
                {
                    writer.WriteLine("Timestamp,UID,TeamColor,Latitude,Longitude,Battery");
                }
            }
        }
        private void LogMarkerToCsv(MarkerViewModel marker)
        {

            using (var writer = new StreamWriter(_csvFilePath, true))
            {
                string line = $"{DateTime.UtcNow:o},{marker.Uid},{marker.TeamColor},{marker.Lat},{marker.Lon},{marker.Battery}";
                writer.WriteLine(line);
            }
        }
        //------------------------




        // PRIMARY METHODS




        //------------------------




        // removes the team color from a marker
        private void RemoveTeamColor(object parameter)
        {
            if (parameter is MarkerViewModel marker)
            {
                marker.TeamColor = "Neutral";
                UpdateMarkerTeamColor(marker, "Neutral"); // no more team, neutral yellow marker
            }
        }

        private void OnDemandExecuted_AddStreamBtn(object sender, EventArgs e)
        {
            Log.i(TAG, MethodBase.GetCurrentMethod() + " enable receiving CoT Message.");
            _cotMessageReceiver.MessageReceived += OnCotMessageReceived;
        }

        public ICommand RemoveTeamColorCommand { get; private set; }

        // handles incoming CoT messages
        private void OnCotMessageReceived(object sender, CoTMessageArgument args)
        {
            Console.WriteLine("OnCotMessageReceived : " + args.Message.ToString());

            var cotEvent = args.CotEvent;
            if (cotEvent != null && cotEvent.Point != null)
            {
                double lat = cotEvent.Point.Latitude;
                double lon = cotEvent.Point.Longitude;
                string uid = cotEvent.Uid;
                string cotType = cotEvent.Type;

                // check if marker already exists
                var existingMarker = MarkerList.FirstOrDefault(m => m.Uid == uid);
                if (existingMarker != null)
                {
                    // update existing marker's position
                    existingMarker.Lat = lat;
                    existingMarker.Lon = lon;
                    existingMarker.Battery = "100%";
                    LogMarkerToCsv(existingMarker);
                }
                else
                {
                    // new marker detected, adding to list
                    var newMarker = new MarkerViewModel
                    {
                        Lat = lat,
                        Lon = lon,
                        Uid = uid,
                        TeamColor = "Neutral",
                        Battery = "100%", // placeholder
                        IsMoving = IsMovementOnByDefault
                    };

                    CoordinatesList.Add(new CoordinateViewModel { Lat = lat, Lon = lon });
                    MarkerList.Add(newMarker);
                    Console.WriteLine($"new marker created at... UID: {newMarker.Uid}, Lat: {newMarker.Lat}, Lon: {newMarker.Lon}");




                    // determine team color from cotType
                    if (!string.IsNullOrEmpty(cotType))
                    {
                        if (cotType.StartsWith("a-h"))
                        {
                            newMarker.TeamColor = "Red";
                        }
                        else if (cotType.StartsWith("a-f"))
                        {
                            newMarker.TeamColor = "Blue";
                        }
                    }

                    // update marker on map
                    UpdateMarkerTeamColor(newMarker, newMarker.TeamColor);
                    LogMarkerToCsv(newMarker);

                    if (newMarker.IsMoving) StartTimerIfNeeded();
                }
            }
            else
            {
                System.Diagnostics.Debug.WriteLine("Received CoT message with no event data.");
            }
        }

        // when the map is clicked...
        private void MapViewControl_MapMouseDown(object sender, MapMouseEventArgs e)
        {
            if (e.Button == MouseButton.Left)
            {
                AddCoordinateAndMarker(e.WorldLocation.Latitude, e.WorldLocation.Longitude, "map-click");

                // we won't add the marker to the map display yet
            }
        }

        // adds coordinate and marker to the lists
        private void AddCoordinateAndMarker(double latitude, double longitude, string source)
        {
            var newCoordinate = new CoordinateViewModel
            {
                Lat = latitude,
                Lon = longitude
            };
            CoordinatesList.Add(newCoordinate);


            _secondLastCoordinate = _lastCoordinate;
            _lastCoordinate = newCoordinate;

            // create a new marker but don't display it yet
            var marker = new MarkerViewModel
            {
                Lat = latitude,
                Lon = longitude,
                TeamColor = "Neutral",
                Uid = MarkerList.Count.ToString(), //so sketch but works
                Battery = "100%",
                IsMoving = IsMovementOnByDefault

                // Uid will be set when the marker is added to the map
            };
            MarkerList.Add(marker);
            Console.WriteLine($"New marker added: Lat: {marker.Lat}, Lon: {marker.Lon}, UID: {marker.Uid}");

            // if we have at least two coordinates, calculate distance
            if (_secondLastCoordinate != null && _lastCoordinate != null)
            {
                double distance = CalculateDistanceInFeet(
                    _secondLastCoordinate.Lat, _secondLastCoordinate.Lon,
                    _lastCoordinate.Lat, _lastCoordinate.Lon);
                CoordinatesList.Add(new CoordinateViewModel
                {
                    Lat = 0,
                    Lon = 0,
                    DisplayText = $"Distance between last two points = {distance:F2} feet" //not necessary but good for perspective and sanity check
                });
            }
            LogMarkerToCsv(marker);
            UpdateMarkerTeamColor(marker, marker.TeamColor);
            if (marker.IsMoving) StartTimerIfNeeded();
        }

        // sets the team color to red
        private void SetRedTeamColor(object parameter)
        {
            if (parameter is MarkerViewModel marker)
            {
                Console.WriteLine(marker.TeamColor);
                marker.TeamColor = "Red";
                UpdateMarkerTeamColor(marker, "Red");
                LogMarkerToCsv(marker);
            }
        }

        // sets the team color to blue
        private void SetBlueTeamColor(object parameter)
        {
            if (parameter is MarkerViewModel marker)
            {
                Console.WriteLine(marker.TeamColor);
                marker.TeamColor = "Blue";
                UpdateMarkerTeamColor(marker, "Blue");
                LogMarkerToCsv(marker);
            }
        }

        // updates the marker's team color on the map
        private void UpdateMarkerTeamColor(MarkerViewModel marker, string teamColor)
        {
            if (_coTManager == null)
            {
                System.Diagnostics.Debug.WriteLine("_coTManager is not initialized.");
                return;
            }

            if (string.IsNullOrEmpty(marker.Uid))
            {
                marker.Uid = Guid.NewGuid().ToString();
            }

            string cotUid = marker.Uid;

            // set CoT type based on team color
            string cotType;
            if (teamColor == "Red")
            {
                cotType = "a-h-G-U-C";
            }
            else if (teamColor == "Blue")
            {
                cotType = "a-f-G-U-C";
            }
            else
            {
                cotType = "b-m-"; //neutral for now, wish we could delete
            }

            // include coordinates and battery percentage in cotName. these are cursor-on-target subschema (additional info)

            string batteryPercentage = "100%"; // dummy value for now
            string cotName = $"Lat: {marker.Lat:F6}, Lon: {marker.Lon:F6}, Battery: {batteryPercentage}";

            // include additional details in cotDetail
            string cotDetail = $"<detail><status battery=\"{batteryPercentage}\" /><contact callsign=\"{cotName}\" /></detail>";

            var location = new TAKEngine.Core.GeoPoint(marker.Lat, marker.Lon);

            double altitude = _elevationManager?.GetElevation(location) ?? Altitude.UNKNOWN_VALUE;
            if (double.IsNaN(altitude))
            {
                altitude = Altitude.UNKNOWN_VALUE;
            }

            var geoPoint = new TAKEngine.Core.GeoPoint(location)
            {
                Altitude = altitude,
                AltitudeRef = TAKEngine.Core.AltitudeReference.HAE
            };

            // send CoT message to display marker on map
            _coTManager.AddItem(cotUid, cotType, geoPoint, cotName, cotDetail);
        }

        // deletes a marker from the list and map
        private void DeleteMarker(object markerObj)
        {
            if (markerObj is MarkerViewModel marker && MarkerList.Contains(marker))
            {
                RemoveMarkerFromMap(marker);
                MarkerList.Remove(marker);
                StopTimerIfNoMarkersMoving();
            }
        }

        // removes marker from the map :eyes:
        // does not truly “remove” the item from the CoT engine’s internal store, just hides it from display client 
        private void RemoveMarkerFromMap(MarkerViewModel marker)
        {
            if (_coTManager == null)
            {
                System.Diagnostics.Debug.WriteLine("_coTManager is not initialized.");
                return;
            }

            if (marker == null) return;

            string uid = marker.Uid;
            if (string.IsNullOrEmpty(uid)) return;

            // hacky, no bad implications I hope
            double Lat = 9999.0;
            double Lon = 9999.0;


            string cotType = "a-n-G"; //neutral bc we dont care. out of sight out of mind
            string cotName = "Marker Hidden";
            string cotDetail = "<detail></detail>";


            var geoPoint = new TAKEngine.Core.GeoPoint(Lat, Lon)
            {
                Altitude = 0,
                AltitudeRef = TAKEngine.Core.AltitudeReference.HAE
            };


            _coTManager.AddItem(
                uid,
                cotType,
                geoPoint,
                cotName,
                cotDetail
            );

            // remove it from  internal list
            MarkerList.Remove(marker);


        }

        // clears the coordinates list
        private void ClearCoordinatesList(object parameter)
        {
            CoordinatesList.Clear();
            _lastCoordinate = null;
            _secondLastCoordinate = null;
        }

        // calculates the distance between two coordinates / haversine formula
        private double CalculateDistanceInFeet(double lat1, double lon1, double lat2, double lon2)
        {
            const double R = 3958; // earth's radius in miles
            double dLat = DegreesToRadians(lat2 - lat1);
            double dLon = DegreesToRadians(lon2 - lon1);

            lat1 = DegreesToRadians(lat1);
            lat2 = DegreesToRadians(lat2);

            double a = Math.Sin(dLat / 2) * Math.Sin(dLat / 2) +
                       Math.Sin(dLon / 2) * Math.Sin(dLon / 2) * Math.Cos(lat1) * Math.Cos(lat2);
            double c = 2 * Math.Atan2(Math.Sqrt(a), Math.Sqrt(1 - a));
            return (R * c) * 5280; // converts miles to feet
        }

        private double DegreesToRadians(double degrees)
        {
            return degrees * Math.PI / 180;
        }

        public new bool IsActive
        {
            get { return _isActive; }
            set { SetProperty(ref _isActive, value, "IsActive"); }
        }

        public ICommand NotifyCommand { get; private set; }
        public ICommand DeleteMarkerCommand { get; private set; }
        public ICommand SetRedTeamColorCommand { get; private set; }
        public ICommand SetBlueTeamColorCommand { get; private set; }
        public ICommand AddMarkerCommand { get; private set; }
        public ICommand ClearCoordinatesCommand { get; private set; }

        public int Counter
        {
            get { return _counter; }
            set { SetProperty(ref _counter, value); }
        }

        private void OnCommandExecuted(object sender, EventArgs e)
        {
            Counter++;
        }

        // adds a marker using manual input
        private void AddMarker(object parameter)
        {
            if (double.TryParse(LatitudeInput, out double lat) && double.TryParse(LongitudeInput, out double lon))
            {
                AddCoordinateAndMarker(lat, lon, "manual input");

                LatitudeInput = string.Empty;
                LongitudeInput = string.Empty;
            }
            else
            {
                System.Diagnostics.Debug.WriteLine("Invalid latitude or longitude input.");
            }
        }

        public string LatitudeInput
        {
            get { return _latitudeInput; }
            set { SetProperty(ref _latitudeInput, value); }
        }
        private string _latitudeInput;

        public string LongitudeInput
        {
            get { return _longitudeInput; }
            set { SetProperty(ref _longitudeInput, value); }
        }
        private string _longitudeInput;

        public class CoordinateViewModel
        {
            public double Lat { get; set; }
            public double Lon { get; set; }

            public string DisplayText
            {
                get { return string.IsNullOrEmpty(_displayText) ? $"{Lat:F6}, {Lon:F6}" : _displayText; }
                set { _displayText = value; }
            }

            private string _displayText;
        }

        public class MarkerViewModel : CoordinateViewModel
        {
            public string Uid { get; set; } // unique identifier from CoT message
            public string TeamColor { get; set; }

            public string Battery { get; set; }

            public bool IsMoving { get; set; }
            public string TeamColorDisplay
            {
                get { return $"Team: {TeamColor}"; }
            }
        }

        //ICommand handlers.. no touch!
        private class ExecutedCommand : ICommand
        {
            private readonly EventHandler _execute;

            public ExecutedCommand(EventHandler execute)
            {
                _execute = execute;
            }

            public event EventHandler CanExecuteChanged;
            public bool CanExecute(object parameter) => true;
            public void Execute(object parameter) => _execute?.Invoke(this, EventArgs.Empty);
        }

        private class RelayCommand : ICommand
        {
            private readonly Action<object> _execute;

            public RelayCommand(Action<object> execute)
            {
                _execute = execute;
            }

            public event EventHandler CanExecuteChanged;
            public bool CanExecute(object parameter) => true;
            public void Execute(object parameter) => _execute(parameter);
        }
    }
}