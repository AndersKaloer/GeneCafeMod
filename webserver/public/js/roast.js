function load_roast_graph(roast) {
    $.ajax({
        type: 'get',
        url: '/roast/data/' + roast.id_urlescaped + '.json',
        dataType: 'json',
        success: function(data) {
            var ctx = document.getElementById("roastChart").getContext('2d');
            var roastChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: data['time'],
                    datasets: [
                        {
                            label: 'Cracks',
                            data: data['data']['crack'],
                            fill: false,
                            showLine: false,
                            pointStyle: 'star',
                            pointRadius: 10,
                            yAxisID: 'tempAxis',
                            pointBorderWidth: 5,
                            borderColor: '#466365',
                            backgroundColor: '#466365'
                        },
                        {
                            label: 'Heat',
                            data: data['data']['heat'],
                            fill: false,
                            yAxisID: 'percentAxis',
                            borderColor: '#F4C95D',
                            backgroundColor: '#F4C95D'
                        },
                        {
                            label: 'Temp.',
                            data: data['data']['temp'],
                            fill: false,
                            yAxisID: 'tempAxis',
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
                                id: 'tempAxis',
                                scaleLabel: {
                                    display: true,
                                    labelString: '°C'
                                },
                                type: 'linear',
                                position: 'left'
                            },
                            {
                                id: 'percentAxis',
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
            'pre_roast_weight' : $('#inputRoastPreWeight').val(),
            'post_roast_weight' : $('#inputRoastPostWeight').val(),
            'rating' : $('#inputRoastRating').val(),
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
