// WeightDao.kt
package com.example.fittrack.data

import androidx.room.*
import kotlinx.coroutines.flow.Flow

// Data access object for handling WeightEntry database operations
@Dao
interface WeightDao {

    // Insert a new weight entry or replace if conflict on primary key
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertWeight(entry: WeightEntry)

    // Delete a specific weight entry
    @Delete
    suspend fun deleteWeight(entry: WeightEntry)

    // Update an existing weight entry
    @Update
    suspend fun updateWeight(entry: WeightEntry)

    // Retrieve all weight entries ordered by date (most recent first)
    @Query("SELECT * FROM weight_entries ORDER BY date DESC")
    fun getAllWeights(): Flow<List<WeightEntry>>
}
