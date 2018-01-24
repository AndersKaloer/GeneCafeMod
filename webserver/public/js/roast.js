// Thanks https://stackoverflow.com/questions/30256695/chart-js-drawing-an-arbitrary-vertical-line/43092029#43092029
const verticalLinePlugin = {
    getLinePosition: function (chart, pointIndex) {
        const meta = chart.getDatasetMeta(0); // first dataset is used to discover X coordinate of a point
        const data = meta.data;
        return data[pointIndex]._model.x;
    },
    renderVerticalLine: function (chartInstance, pointIndex) {
        const lineLeftOffset = this.getLinePosition(chartInstance, pointIndex);
        const scale = chartInstance.scales['y-axis-0'];
        const context = chartInstance.chart.ctx;

        // render vertical line
        context.beginPath();
        context.strokeStyle = '#ff0000';
        context.moveTo(lineLeftOffset, scale.top);
        context.lineTo(lineLeftOffset, scale.bottom);
        context.stroke();

        // write label
        context.fillStyle = "#ff0000";
        context.textAlign = 'right';
        context.fillText('Crack', lineLeftOffset-5, (scale.bottom - scale.top) / 2 + scale.top);
    },

    afterDatasetsDraw: function (chart, easing) {
        if (chart.config.lineAtIndex) {
            chart.config.lineAtIndex.forEach(pointIndex => this.renderVerticalLine(chart, pointIndex));
        }
    }
};

Chart.plugins.register(verticalLinePlugin);

function load_roast_graph(roast) {
    $.ajax({
        type: 'get',
        url: '/roast/data/' + roast.id_urlescaped + '.json',
        dataType: 'json',
        success: function(data) {
            var ctx = document.getElementById("roastChart").getContext('2d');
            var roastChart = new Chart(ctx, {
                type: 'line',
                lineAtIndex: data['crack'],
                data: {
                    labels: data['time'],
                    datasets: [
                        {
                            label: 'Heat',
                            data: data['data']['heat'],
                            fill: false,
                            yAxisID: 'y-axis-1',
                            borderColor: '#F4C95D',
                            backgroundColor: '#F4C95D'
                        },
                        {
                            label: 'Temp.',
                            data: data['data']['temp'],
                            fill: false,
                            yAxisID: 'y-axis-0',
                            borderColor: '#DD7230',
                            backgroundColor: '#DD7230'
                        }]
                },
                options: {
                    tooltips: {
                        enabled: false
                    },
                    scales: {
                        xAxes: [{
                            scaleLabel: {display:true, labelString: 'Time'},
                            type: 'time',
                            time: {
                                unit: 'minute',
                                displayFormats: {
                                    minute: 'mm:ss'
                                }
                            },
                        }],
                        yAxes: [
                            {
                                id: 'y-axis-0',
                                scaleLabel: {
                                    display: true,
                                    labelString: '°C'
                                },
                                type: 'linear',
                                position: 'left'
                            },
                            {
                                id: 'y-axis-1',
                                scaleLabel: {
                                    display: true,
                                    labelString: '%'
                                },
                                type: 'linear',
                                position: 'right'
                            }
                        ]                            
                    }
                }
            });
        }
    })
}


$(document).ready(function() {
    $("#roast_desc_form").submit(function(event) {
        $("#roast_desc_form button[type=submit]").prop("disabled", true);
        $("#submit_spinner").show();
        
        var newRoast = {
            'coffee': $('#inputCoffee').val(),
            'roast_degree' : $('#inputRoastDegree').val(),
            'pre_roast_weight' : parseInt($('#inputRoastPreWeight').val()),
            'post_roast_weight' : parseInt($('#inputRoastPostWeight').val()),
            'rating' : parseInt($('#inputRoastRating').val()),
            'comment' : $('#inputRoastComment').val()
        };
        $.ajax({
            type: 'POST',
            url: '/roast/' + roast.id_urlescaped,
            data: newRoast,
            encode: true
        }).success(function() {
            $("#roast_desc_form button[type=submit]").prop("disabled", false);
            $("#submit_spinner").hide();
        });
        event.preventDefault();
    });

    $("#delete_btn").click(function(event) {
        if(confirm("Are you sure?")) {
            $("#delete_btn").prop("disabled", true);
            $("#delete_spinner").show();
            $.ajax({
                type: 'DELETE',
                url: '/roast/' + roast.id_urlescaped,
                encode: true
            }).success(function() {
                $("#delete_btn").prop("disabled", false);
                $("#delete_spinner").hide();
                window.location.replace("/");
            });
        }
    });
});
