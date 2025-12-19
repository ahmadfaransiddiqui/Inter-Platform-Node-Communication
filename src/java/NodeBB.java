import java.io.*;
import java.net.*;

public class NodeBB {
    public static void main(String[] args) {
        int port = 12345;
        String serverIP = "192.168.1.5";  // Replace with Node A's IP address
        int timeoutSeconds = 5;  // Timeout duration in seconds

        try (Socket clientSocket = new Socket(serverIP, port);
             BufferedReader userInput = new BufferedReader(new InputStreamReader(System.in));
             BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
             PrintWriter writer = new PrintWriter(clientSocket.getOutputStream(), true)) {

            clientSocket.setSoTimeout(timeoutSeconds * 1000);  // Set timeout in milliseconds

            System.out.println("Connected to Node A");

            while (true) {
                displayMenu();
                System.out.print("Enter your choice: ");
                String choice = userInput.readLine().toLowerCase();  // Convert input to lowercase

                if (choice.equals("exit")) {
                    break;
                } else {
                    writer.println(choice);

                    if (choice.equals("write")) {
                        System.out.print("Enter data to write: ");
                        String data = userInput.readLine();
                        writer.println(data);  // Send data separately for write
                        continue;  // Skip waiting for a response for write
                    }

                    try {
                        String response = reader.readLine();
                        if (response != null) {
                            System.out.println("Response from Node A: " + response);
                        } else {
                            System.out.println("No response from Node A.");
                        }
                    } catch (SocketTimeoutException e) {
                        System.out.println("Timeout: No response from Node A.");
                    }
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }

        System.out.println("Disconnected from Node A");
    }

    private static void displayMenu() {
        System.out.println("\nMenu:");
        System.out.println("1. Read data from shared memory");
        System.out.println("2. Write data to shared memory");
        System.out.println("3. Exit\n");
    }
}
