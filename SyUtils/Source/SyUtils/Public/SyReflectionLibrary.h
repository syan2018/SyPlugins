#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SyReflectionLibrary.generated.h"

/**
 * Library for Generic Reflection and Property Access in Blueprints.
 */
UCLASS()
class SYUTILS_API USyReflectionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Struct Access (Wildcard)
	// ========================================================================

	/**
	 * Try to find a property in a Struct by name and return it as a String.
	 * Useful for debug or generic UI.
	 * 
	 * @param TargetStruct The struct to read from. (Connect any struct here)
	 * @param PropertyName The name of the member variable.
	 * @param OutStringValue The resulting value as string.
	 * @return True if property found.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "SyUtils|Reflection", meta = (CustomStructureParam = "TargetStruct", DisplayName = "Get Struct Property (String)"))
	static bool GetStructPropertyAsString(const int32& TargetStruct, FName PropertyName, FString& OutStringValue);
	
	DECLARE_FUNCTION(execGetStructPropertyAsString);

	/**
	 * Try to find a property in a Struct by name and return it as an Integer.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "SyUtils|Reflection", meta = (CustomStructureParam = "TargetStruct", DisplayName = "Get Struct Property (Int)"))
	static bool GetStructPropertyAsInt(const int32& TargetStruct, FName PropertyName, int32& OutIntValue);

	DECLARE_FUNCTION(execGetStructPropertyAsInt);
    
	/**
	 * Try to find a property in a Struct by name and return it as a Float.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "SyUtils|Reflection", meta = (CustomStructureParam = "TargetStruct", DisplayName = "Get Struct Property (Float)"))
	static bool GetStructPropertyAsFloat(const int32& TargetStruct, FName PropertyName, double& OutFloatValue); // double for BP Float

	DECLARE_FUNCTION(execGetStructPropertyAsFloat);

	/**
	 * Try to find a property in a Struct by name and return it as another Struct.
	 * The Output Struct type is determined by the pin connection.
	 * 
	 * @param OutStructType Returns the actual type of the struct found (even if copy failed or type mismatched).
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "SyUtils|Reflection", meta = (CustomStructureParam = "TargetStruct,OutStructValue", DisplayName = "Get Struct Property (Struct)"))
	static bool GetStructPropertyAsStruct(const int32& TargetStruct, FName PropertyName, int32& OutStructValue, UScriptStruct*& OutStructType);

	DECLARE_FUNCTION(execGetStructPropertyAsStruct);


	// ========================================================================
	// DataTable Access (Row Name + Column Name)
	// ========================================================================

	/**
	 * Get a value from a DataTable using Row Name and Column (Property) Name.
	 * 
	 * @param DataTable The table to search.
	 * @param RowName The row key.
	 * @param ColumnName The variable name of the column.
	 * @param OutStringValue The value converted to string.
	 * @return True if found.
	 */
	UFUNCTION(BlueprintCallable, Category = "SyUtils|DataTable", meta = (DisplayName = "Get Data Table Value (String)"))
	static bool GetDataTableValueAsString(UDataTable* DataTable, FName RowName, FName ColumnName, FString& OutStringValue);

	/**
	 * Get an Integer value from a DataTable using Row Name and Column Name.
	 */
	UFUNCTION(BlueprintCallable, Category = "SyUtils|DataTable", meta = (DisplayName = "Get Data Table Value (Int)"))
	static bool GetDataTableValueAsInt(UDataTable* DataTable, FName RowName, FName ColumnName, int32& OutIntValue);

	/**
	 * Get a Float value from a DataTable using Row Name and Column Name.
	 */
	UFUNCTION(BlueprintCallable, Category = "SyUtils|DataTable", meta = (DisplayName = "Get Data Table Value (Float)"))
	static bool GetDataTableValueAsFloat(UDataTable* DataTable, FName RowName, FName ColumnName, double& OutFloatValue);

	/**
	 * Get a Struct value from a DataTable using Row Name and Column Name.
	 * 
	 * @param OutStructType Returns the actual type of the struct found in the table.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "SyUtils|DataTable", meta = (CustomStructureParam = "OutStructValue", DisplayName = "Get Data Table Value (Struct)"))
	static bool GetDataTableValueAsStruct(UDataTable* DataTable, FName RowName, FName ColumnName, int32& OutStructValue, UScriptStruct*& OutStructType);

	DECLARE_FUNCTION(execGetDataTableValueAsStruct);


	// ========================================================================
	// DataTable Column Access (Column Map)
	// ========================================================================

	/**
	 * Extract a whole column from the DataTable as a Map where Key = RowName and Value = Column Value.
	 * The value type of the map determines the expected column property type.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "SyUtils|DataTable", meta = (CustomStructureParam = "OutColumnMap", MapParam = "OutColumnMap", DisplayName = "Get Data Table Column (Map)"))
	static bool GetDataTableColumnAsMap(UDataTable* DataTable, FName ColumnName, UPARAM(ref) TMap<FName, int32>& OutColumnMap);

	DECLARE_FUNCTION(execGetDataTableColumnAsMap);


	// ========================================================================
	// Helpers
	// ========================================================================
	
	static bool Generic_GetPropertyAsString(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, FString& OutValue);
	static bool Generic_GetPropertyAsInt(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, int32& OutValue);
	static bool Generic_GetPropertyAsFloat(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, double& OutValue);
	static bool Generic_GetPropertyAsStruct(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, const UScriptStruct* OutStructDef, void* OutStructPtr, UScriptStruct*& OutFoundStructType);
	static bool Generic_GetDataTableColumnAsMap(UDataTable* DataTable, FName ColumnName, const FMapProperty* OutputMapProp, void* OutputMapPtr);
};
