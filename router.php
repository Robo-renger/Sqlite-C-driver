<?php

// Get the message parameter from the JavaScript request
$message = $_GET['message'];

// Validate and sanitize the input (you may want to do more robust validation)
if (empty($message)) {
    header('HTTP/1.1 400 Bad Request');
    echo json_encode(['error' => 'Message parameter is required']);
    exit;
}
// Execute the main C program and pass the message as a parameter
$output = shell_exec('./main ' . $message);

// Send the output as a JSON response
header('Content-Type: application/json');
echo json_encode(['output' => $output]);

?>
