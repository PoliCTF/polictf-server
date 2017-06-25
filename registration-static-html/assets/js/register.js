
function feedbackError(htmlText) {
    $("#reg-feedback").attr("class", "alert alert-danger");
    $("#reg-feedback").html('<span class="fa fa-exclamation" aria-hidden="true"></span><span class="sr-only">Error:</span><span id="reg-error"></span>');
    $("#reg-error").html(htmlText);
    $("html, body").animate({ scrollTop: 0 }, "slow");
}

function feedbackOk(htmlText) {
    $("#reg-feedback").attr("class", "alert alert-info");
    $("#reg-feedback").html('<span id="reg-error"></span>');
    $("#reg-error").html(htmlText);
    $("html, body").animate({ scrollTop: 0 }, "slow");
}

$(document).ready(function() {

    var params = {};
    if(location.hash) {
        params = location.hash.split("#")[1].split("&").map( el => el.split("=") ).reduce( (pre, cur) => { pre[cur[0]] = cur[1]; return pre; }, {} );
    }
    console.log(params);
    if(params['email'] && params['token']) {
        $('#reset-form').show();
        $('#reset-init-form').hide();
        $('#reset-name').text(params['email']);
        $('#reset-form').validator().on('submit', function (e) {
            if (!e.isDefaultPrevented()) {
                $("#reg-error").removeClass();
                $("#reg-error").text("");
                // everything looks good!
                e.preventDefault();
                // Use Ajax to submit form data
                $.ajax({
                    method: "POST",
                    url: "https://registration.polictf.it/api/reset/token",
                    data: { email: params['email'],
                            password: $("#password_first").val(),
                            token: params['token']
                    },
                    success: function(data_r) {
                        if(data_r.done) {
                            feedbackOk("&nbsp;Password reset successfully.")
                            $("#reset-form").hide()
                        }
                        else {
                            var error_t = "";
                            for (error_elm in data_r.errors) {
                                error_t = error_t + "<br /> - " + data_r.errors[error_elm].trim();
                            }
                            feedbackError("&nbsp;Some data isn't correct: <div style=\"font-size: 9pt;\">" + error_t + "</div>" );
                        }
                    },
                    error: function() { 
                       feedbackError("&nbsp;Technical error. Please retry later.");
                    }
                });
            }
        });
    } else {
        $('#reset-form').hide();
        $('#reset-init-form').show();
    }

    $('#reg-form').validator().on('submit', function (e) {
        if (!e.isDefaultPrevented()) {
    		$("#reg-error").removeClass();
    		$("#reg-error").text("");
            // everything looks good!
            e.preventDefault();
            // Use Ajax to submit form data
            $.ajax({
                method: "POST",
                url: "https://registration.polictf.it/api/subscribe",
                data: { name:  $("#name").val(), 
                        email: $("#email").val(),
                        password: $("#password").val(),
                        rules: $("#check_rules").is(":checked") ? "y" : "n",
                        country: $("span.bfh-selectbox-option").text(),
                        website: $("#website").val(),
                        twitter: $("#twitter").val()
                },
                success: function(data_r) {
    				if(data_r.done) {
                        feedbackOk("<b>Registration successful!</b><br /> We sent you an e-mail to confirm the registration. <br/> If you don't receive it, please check your junk e-mail folder.")
    					$("#reg-form").hide()
    				}
    				else {
    					var error_t = "";
    					for (error_elm in data_r.errors) {
    						error_t = error_t + "<br /> - " + data_r.errors[error_elm].trim();
    					}
                        feedbackError("&nbsp;Some data isn't correct: <div style=\"font-size: 9pt;\">" + error_t + "</div>" );
    				}
                },
                error: function() { 
                   feedbackError("&nbsp;Technical error. Please retry later.");
                }
            });
        }
    });

    $('#reset-init-form').validator().on('submit', function (e) {
        if (!e.isDefaultPrevented()) {
            $("#reg-error").removeClass();
            $("#reg-error").text("");
            // everything looks good!
            e.preventDefault();
            // Use Ajax to submit form data
            $.ajax({
                method: "POST",
                url: "https://registration.polictf.it/api/reset/init",
                data: { email: $("#email_address").val()
                },
                success: function(data_r) {
                    if(data_r.done) {
                        feedbackOk("&nbsp;We sent you an e-mail with further instructions.")
                        $("#reset-init-form").hide()
                    }
                    else {
                        var error_t = "";
                        for (error_elm in data_r.errors) {
                            error_t = error_t + "<br /> - " + data_r.errors[error_elm].trim();
                        }
                        feedbackError("&nbsp;Some data isn't correct: <div style=\"font-size: 9pt;\">" + error_t + "</div>" );
                    }
                },
                error: function() { 
                   feedbackError("&nbsp;Technical error. Please retry later.");
                }
            });
        }
    });

});
