using LiveCharts;
using LiveCharts.Defaults;
using LiveCharts.Wpf;
using System.Windows;

namespace Client;

public class ChartManager
{
    private readonly CartesianChart _chart;

    public ChartManager(CartesianChart chart)
    {
        _chart = chart;
        _chart.Series = new SeriesCollection();
    }

    public void AddChart(string graphTitle, List<Point> dataPoints)
    {
        var chartValues = new ChartValues<ObservablePoint>(
            dataPoints.Select(p => new ObservablePoint(p.X, p.Y))
        );

        var lineSeries = new LineSeries
        {
            Title = graphTitle,
            Values = chartValues,
            PointGeometry = null,
            LineSmoothness = 1,
            Fill = System.Windows.Media.Brushes.Transparent
        };

        _chart.Series.Add(lineSeries);

        if (_chart.AxisX.Count == 0)
        {
            _chart.AxisX.Add(new Axis
            {
                Title = "X Axis",
                Separator = new Separator { Step = 1 }
            });
        }

        _chart.Update(true, true);
    }

    public void ClearCharts()
    {
        _chart.Series.Clear();
        _chart.Update(true, true);
    }
}
