// User.kt
package com.example.fittrack.data

import androidx.room.Entity
import androidx.room.PrimaryKey

// Represents a user in the database
@Entity(tableName = "users")
data class User(
    @PrimaryKey val username: String, // Unique identifier for each user
    val passwordHash: String // User's password
)
