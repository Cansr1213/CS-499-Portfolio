// AppDatabase.kt
package com.example.fittrack.data

import android.content.Context
import androidx.room.Database
import androidx.room.Room
import androidx.room.RoomDatabase

// Room database setup with User and WeightEntry entities
@Database(entities = [User::class, WeightEntry::class], version = 2)
abstract class AppDatabase : RoomDatabase() {
    abstract fun userDao(): UserDao // Accessor for User operations
    abstract fun weightDao(): WeightDao // Accessor for WeightEntry operations

    companion object {
        @Volatile
        private var INSTANCE: AppDatabase? = null // Singleton instance

        // Provides singleton instance of the database
        fun getDatabase(context: Context): AppDatabase {
            return INSTANCE ?: synchronized(this) {
                val instance = Room.databaseBuilder(
                    context.applicationContext,
                    AppDatabase::class.java,
                    "fittrack_db"
                )
                    .fallbackToDestructiveMigration() // Clears DB if schema changes
                    .build()
                INSTANCE = instance
                instance
            }
        }
    }
}
