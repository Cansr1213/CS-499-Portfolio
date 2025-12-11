// WeightEntry.kt
package com.example.fittrack.data

import androidx.room.Entity
import androidx.room.Index
import androidx.room.PrimaryKey

// Defines a Room entity for to store weight tracking entries per user
@Entity(
    tableName = "weight_entries",
    indices = [
        Index(value = ["userUsername"]),
        Index(value = ["date"])
    ]
)
data class WeightEntry(
    @PrimaryKey(autoGenerate = true) val id: Int = 0,
    val userUsername: String,   // which user this entry belongs to
    val date: Long,         // timestamp in millis (System.currentTimeMillis())
    val weight: Double      // numeric weight for easier calculations
)