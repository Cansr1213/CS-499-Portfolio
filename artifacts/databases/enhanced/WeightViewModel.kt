package com.example.fittrack.ui.viewmodels   // ✅ MUST match folder

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.fittrack.data.AppDatabase
import com.example.fittrack.data.WeightEntry
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch

class WeightViewModel(
    private val db: AppDatabase,
    private val username: String
) : ViewModel() {

    // Automatically updates UI when database changes
    val weights: StateFlow<List<WeightEntry>> =
        db.weightDao().getWeightsForUser(username)
            .stateIn(
                scope = viewModelScope,
                started = SharingStarted.WhileSubscribed(5000),
                initialValue = emptyList()
            )

    // Save (insert or update)
    fun saveWeight(existing: WeightEntry?, weight: Double) {
        viewModelScope.launch(Dispatchers.IO) {
            val now = System.currentTimeMillis()

            if (existing == null) {
                // Create new entry
                db.weightDao().insertWeight(
                    WeightEntry(
                        userUsername = username,
                        date = now,
                        weight = weight
                    )
                )
            } else {
                // Update entry
                db.weightDao().updateWeight(
                    existing.copy(weight = weight)
                )
            }
        }
    }

    // Delete entry
    fun deleteWeight(entry: WeightEntry) {
        viewModelScope.launch(Dispatchers.IO) {
            db.weightDao().deleteWeight(entry)
        }
    }
}
