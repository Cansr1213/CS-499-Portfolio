// MainActivity.kt
package com.example.fittrack

import android.Manifest
import android.os.Build
import android.os.Bundle
import android.telephony.SmsManager
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.fittrack.data.AppDatabase
import com.example.fittrack.data.User
import com.example.fittrack.data.WeightEntry
import com.example.fittrack.ui.theme.FitTrackTheme
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

class MainActivity : ComponentActivity() {
    private lateinit var db: AppDatabase

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        db = AppDatabase.getDatabase(applicationContext) // Initialize database

        setContent {
            FitTrackTheme {
                Surface(modifier = Modifier.fillMaxSize()) {
                    AppScreens(db) // Launch main UI screen
                }
            }
        }
    }
}

@Composable
fun AppScreens(db: AppDatabase) {
    var currentScreen by remember { mutableStateOf("login") } // Tracks current screen
    var loginError by remember { mutableStateOf("") } // Error message holder

    when (currentScreen) {
        "login" -> LoginScreen(
            loginError = loginError,
            onLogin = { username, password ->
                CoroutineScope(Dispatchers.IO).launch {
                    val user = db.userDao().getUser(username, password) // Check credentials
                    loginError = if (user != null) {
                        currentScreen = "weight"
                        ""
                    } else {
                        "Invalid credentials"
                    }
                }
            },
            onCreateAccount = { username, password ->
                CoroutineScope(Dispatchers.IO).launch {
                    if (username.isBlank() || password.isBlank()) {
                        loginError = "Username and password cannot be blank."
                    } else if (db.userDao().getUserByUsername(username) != null) {
                        loginError = "Username already exists."
                    } else {
                        db.userDao().insertUser(User(username = username, password = password))
                        loginError = "Account created successfully."
                    }
                }
            },
            onProceed = { currentScreen = "weight" } // For skipping login
        )

        "weight" -> WeightScreen(db) { currentScreen = "sms" } // Navigate to weight screen
        "sms" -> SmsPermissionScreen() // Navigate to SMS permission screen
    }
}

@Composable
fun LoginScreen(
    loginError: String,
    onLogin: (String, String) -> Unit,
    onCreateAccount: (String, String) -> Unit,
    onProceed: () -> Unit
) {
    var userName by remember { mutableStateOf("") }
    var password by remember { mutableStateOf("") }

    Column(
        modifier = Modifier.fillMaxSize().padding(32.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text("FitTrack Login", fontSize = 26.sp, fontWeight = FontWeight.Bold)

        OutlinedTextField(
            value = userName,
            onValueChange = { userName = it },
            label = { Text("Username") },
            singleLine = true,
            modifier = Modifier.fillMaxWidth()
        )

        OutlinedTextField(
            value = password,
            onValueChange = { password = it },
            label = { Text("Password") },
            visualTransformation = PasswordVisualTransformation(),
            singleLine = true,
            modifier = Modifier.fillMaxWidth()
        )

        if (loginError.isNotBlank()) {
            Text(text = loginError, color = MaterialTheme.colorScheme.error)
        }

        Button(onClick = { onLogin(userName, password) }, modifier = Modifier.fillMaxWidth()) {
            Text("Login")
        }

        Button(onClick = { onCreateAccount(userName, password) }, modifier = Modifier.fillMaxWidth()) {
            Text("Create Account")
        }

        Button(onClick = onProceed, modifier = Modifier.fillMaxWidth()) {
            Text("Continue")
        }
    }
}

@Composable
fun WeightScreen(db: AppDatabase, onContinue: () -> Unit) {
    var weightInput by remember { mutableStateOf("") }
    val entries = remember { mutableStateListOf<WeightEntry>() } // Holds weight data
    var isEditing by remember { mutableStateOf(false) }
    var editEntry by remember { mutableStateOf<WeightEntry?>(null) } // Currently editing entry

    // Load weight data
    LaunchedEffect(Unit) {
        db.weightDao().getAllWeights().collect { data ->
            entries.clear()
            entries.addAll(data)
        }
    }

    Column(
        modifier = Modifier.fillMaxSize().padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text("Track Your Weight", fontSize = 26.sp, fontWeight = FontWeight.Bold)

        OutlinedTextField(
            value = weightInput,
            onValueChange = { weightInput = it },
            label = { Text("Enter Weight (lbs)") },
            singleLine = true,
            modifier = Modifier.fillMaxWidth()
        )

        Button(onClick = {
            if (weightInput.isNotBlank()) {
                val currentDate = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(Date())
                CoroutineScope(Dispatchers.IO).launch {
                    if (isEditing && editEntry != null) {
                        val updated = editEntry!!.copy(weight = weightInput)
                        db.weightDao().updateWeight(updated)
                        entries[entries.indexOf(editEntry!!)] = updated
                        isEditing = false
                        editEntry = null
                    } else {
                        val newEntry = WeightEntry(date = currentDate, weight = weightInput)
                        db.weightDao().insertWeight(newEntry)
                        entries.add(0, newEntry)
                    }
                    weightInput = ""
                }
            }
        }, modifier = Modifier.fillMaxWidth()) {
            Text(if (isEditing) "Update Weight" else "Add Weight")
        }

        Button(onClick = onContinue, modifier = Modifier.fillMaxWidth()) {
            Text("Continue to SMS Setup")
        }

        Text("History", fontSize = 20.sp, fontWeight = FontWeight.SemiBold)

        LazyColumn(modifier = Modifier.fillMaxSize()) {
            items(entries) { entry ->
                Card(
                    modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp),
                    elevation = CardDefaults.elevatedCardElevation(defaultElevation = 4.dp)
                ) {
                    Row(
                        modifier = Modifier.padding(12.dp).fillMaxWidth(),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Column {
                            Text("Date: ${entry.date}")
                            Text("Weight: ${entry.weight} lbs")
                        }
                        Row {
                            Button(onClick = {
                                weightInput = entry.weight
                                editEntry = entry
                                isEditing = true
                            }) {
                                Text("Edit")
                            }
                            Button(onClick = {
                                CoroutineScope(Dispatchers.IO).launch {
                                    db.weightDao().deleteWeight(entry)
                                    entries.remove(entry)
                                }
                            }) {
                                Text("Delete")
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun SmsPermissionScreen() {
    val context = LocalContext.current
    var permissionGranted by remember { mutableStateOf(false) } // Tracks permission state

    val smsPermissionLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted ->
        permissionGranted = isGranted

        if (isGranted) {
            Toast.makeText(context, "SMS permission granted", Toast.LENGTH_SHORT).show()

            val hasTelephony = context.packageManager.hasSystemFeature("android.hardware.telephony")

            val smsManager: SmsManager? = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                context.getSystemService(SmsManager::class.java)
            } else {
                @Suppress("DEPRECATION")
                SmsManager.getDefault()
            }

            if (smsManager != null && hasTelephony) {
                try {
                    smsManager.sendTextMessage(
                        "555-123-4567", null,
                        "Goal reached! Congrats on your progress!", null, null
                    )
                    Toast.makeText(context, "Test SMS sent", Toast.LENGTH_SHORT).show()
                } catch (e: Exception) {
                    Toast.makeText(context, "SMS failed: ${e.message}", Toast.LENGTH_LONG).show()
                }
            } else {
                Toast.makeText(context, "SMS not supported on this device", Toast.LENGTH_LONG).show()
            }
        } else {
            Toast.makeText(context, "SMS permission denied", Toast.LENGTH_SHORT).show()
        }
    }

    Column(
        modifier = Modifier.fillMaxSize().padding(32.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Text("SMS Permission Setup", fontSize = 24.sp, fontWeight = FontWeight.Bold)
        Text("To receive alerts when goals are reached, grant SMS permission.")
        Button(onClick = {
            smsPermissionLauncher.launch(Manifest.permission.SEND_SMS)
        }) {
            Text("Grant Permission")
        }
    }
}
