function makeAjaxRequest() {
    // Message to be sent to the PHP script
    var message = "getAllAccounts";

    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/core/router.php?message=" + encodeURIComponent(message), true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            var response = JSON.parse(xhr.responseText);
            document.getElementById("accountTable").innerHTML = "Output: " + response.output;
        } else if (xhr.readyState == 4 && xhr.status != 200) {
            document.getElementById("result").innerHTML = "Error: " + xhr.status;
        }
    };
    xhr.send();
}
makeAjaxRequest()