#include "SyReflectionLibrary.h"
#include "Blueprint/BlueprintExceptionInfo.h"
#include "Engine/DataTable.h"

// ============================================================================
// Custom Thunks (Struct Access)
// ============================================================================

DEFINE_FUNCTION(USyReflectionLibrary::execGetStructPropertyAsString)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;

	// Read the wildcard struct input
	Stack.StepCompiledIn<FStructProperty>(NULL);
	void* StructAddr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FNameProperty, PropertyName);
	P_GET_PROPERTY_REF(FStrProperty, OutValue);

	P_FINISH;

	if (!StructProp || !StructAddr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			NSLOCTEXT("SyUtils", "MissingStruct", "Failed to resolve struct input.")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		*(bool*)RESULT_PARAM = false;
		return;
	}

	P_NATIVE_BEGIN;
	*(bool*)RESULT_PARAM = Generic_GetPropertyAsString(StructProp->Struct, StructAddr, PropertyName, OutValue);
	P_NATIVE_END;
}

DEFINE_FUNCTION(USyReflectionLibrary::execGetStructPropertyAsInt)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;

	Stack.StepCompiledIn<FStructProperty>(NULL);
	void* StructAddr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FNameProperty, PropertyName);
	P_GET_PROPERTY_REF(FIntProperty, OutValue);

	P_FINISH;

	if (!StructProp || !StructAddr)
	{
		*(bool*)RESULT_PARAM = false;
		return;
	}

	P_NATIVE_BEGIN;
	*(bool*)RESULT_PARAM = Generic_GetPropertyAsInt(StructProp->Struct, StructAddr, PropertyName, OutValue);
	P_NATIVE_END;
}

DEFINE_FUNCTION(USyReflectionLibrary::execGetStructPropertyAsFloat)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;

	Stack.StepCompiledIn<FStructProperty>(NULL);
	void* StructAddr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FNameProperty, PropertyName);
	P_GET_PROPERTY_REF(FDoubleProperty, OutValue);

	P_FINISH;

	if (!StructProp || !StructAddr)
	{
		*(bool*)RESULT_PARAM = false;
		return;
	}

	P_NATIVE_BEGIN;
	*(bool*)RESULT_PARAM = Generic_GetPropertyAsFloat(StructProp->Struct, StructAddr, PropertyName, OutValue);
	P_NATIVE_END;
}

DEFINE_FUNCTION(USyReflectionLibrary::execGetStructPropertyAsStruct)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;

	// 1. Input Struct
	Stack.StepCompiledIn<FStructProperty>(NULL);
	void* InputStructAddr = Stack.MostRecentPropertyAddress;
	FStructProperty* InputStructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	// 2. Property Name
	P_GET_PROPERTY(FNameProperty, PropertyName);

	// 3. Output Struct (Ref) - Step over the output pin
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(NULL);
	void* OutputStructAddr = Stack.MostRecentPropertyAddress;
	FStructProperty* OutputStructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	// 4. Output Struct Type (Ref)
	P_GET_OBJECT_REF(UScriptStruct, OutStructType);

	P_FINISH;

	if (!InputStructProp || !InputStructAddr || !OutputStructProp || !OutputStructAddr)
	{
		*(bool*)RESULT_PARAM = false;
		return;
	}

	P_NATIVE_BEGIN;
	*(bool*)RESULT_PARAM = Generic_GetPropertyAsStruct(InputStructProp->Struct, InputStructAddr, PropertyName, OutputStructProp->Struct, OutputStructAddr, OutStructType);
	P_NATIVE_END;
}

DEFINE_FUNCTION(USyReflectionLibrary::execGetDataTableValueAsStruct)
{
	P_GET_OBJECT(UDataTable, DataTable);
	P_GET_PROPERTY(FNameProperty, RowName);
	P_GET_PROPERTY(FNameProperty, ColumnName);
	
	// Step over output struct
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(NULL);
	void* OutputStructAddr = Stack.MostRecentPropertyAddress;
	FStructProperty* OutputStructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	// Step over Output Struct Type
	P_GET_OBJECT_REF(UScriptStruct, OutStructType);

	P_FINISH;

	if (!DataTable || !OutputStructProp || !OutputStructAddr)
	{
		*(bool*)RESULT_PARAM = false;
		return;
	}

	// Find Row
	uint8* RowData = DataTable->FindRowUnchecked(RowName);
	if (!RowData)
	{
		*(bool*)RESULT_PARAM = false;
		return;
	}

	P_NATIVE_BEGIN;
	*(bool*)RESULT_PARAM = Generic_GetPropertyAsStruct(DataTable->RowStruct, RowData, ColumnName, OutputStructProp->Struct, OutputStructAddr, OutStructType);
	P_NATIVE_END;
}


// ============================================================================
// DataTable Access
// ============================================================================

bool USyReflectionLibrary::GetDataTableValueAsString(UDataTable* DataTable, FName RowName, FName ColumnName, FString& OutStringValue)
{
	if (!DataTable) return false;

	// Find the row data
	uint8* RowData = DataTable->FindRowUnchecked(RowName);
	if (!RowData) return false;

	return Generic_GetPropertyAsString(DataTable->RowStruct, RowData, ColumnName, OutStringValue);
}

bool USyReflectionLibrary::GetDataTableValueAsInt(UDataTable* DataTable, FName RowName, FName ColumnName, int32& OutIntValue)
{
	if (!DataTable) return false;

	uint8* RowData = DataTable->FindRowUnchecked(RowName);
	if (!RowData) return false;

	return Generic_GetPropertyAsInt(DataTable->RowStruct, RowData, ColumnName, OutIntValue);
}

bool USyReflectionLibrary::GetDataTableValueAsFloat(UDataTable* DataTable, FName RowName, FName ColumnName, double& OutFloatValue)
{
	if (!DataTable) return false;

	uint8* RowData = DataTable->FindRowUnchecked(RowName);
	if (!RowData) return false;

	return Generic_GetPropertyAsFloat(DataTable->RowStruct, RowData, ColumnName, OutFloatValue);
}


// ============================================================================
// Helpers (The Core Logic)
// ============================================================================

bool USyReflectionLibrary::Generic_GetPropertyAsString(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, FString& OutValue)
{
	if (!StructDef || !ContainerPtr) return false;

	if (const FProperty* Property = StructDef->FindPropertyByName(PropertyName))
	{
		// Convert value to string using ExportText
		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ContainerPtr);
		Property->ExportTextItem_Direct(OutValue, ValuePtr, ValuePtr, nullptr, PPF_None);
		return true;
	}
	return false;
}

bool USyReflectionLibrary::Generic_GetPropertyAsInt(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, int32& OutValue)
{
	if (!StructDef || !ContainerPtr) return false;

	const FProperty* Property = StructDef->FindPropertyByName(PropertyName);
	if (!Property) return false;

	// Handle Integer Types
	if (const FIntProperty* IntProp = CastField<FIntProperty>(Property))
	{
		OutValue = IntProp->GetPropertyValue_InContainer(ContainerPtr);
		return true;
	}
	else if (const FInt64Property* Int64Prop = CastField<FInt64Property>(Property))
	{
		// Possible data loss warning? For now just cast.
		OutValue = (int32)Int64Prop->GetPropertyValue_InContainer(ContainerPtr);
		return true;
	}
	else if (const FByteProperty* ByteProp = CastField<FByteProperty>(Property))
	{
		OutValue = (int32)ByteProp->GetPropertyValue_InContainer(ContainerPtr);
		return true;
	}
	
	return false;
}

bool USyReflectionLibrary::Generic_GetPropertyAsFloat(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, double& OutValue)
{
	if (!StructDef || !ContainerPtr) return false;

	const FProperty* Property = StructDef->FindPropertyByName(PropertyName);
	if (!Property) return false;

	// Handle Float/Double Types
	if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property))
	{
		OutValue = DoubleProp->GetPropertyValue_InContainer(ContainerPtr);
		return true;
	}
	else if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
	{
		OutValue = (double)FloatProp->GetPropertyValue_InContainer(ContainerPtr);
		return true;
	}
	else if (const FIntProperty* IntProp = CastField<FIntProperty>(Property))
	{
		// Allow reading Int as Float
		OutValue = (double)IntProp->GetPropertyValue_InContainer(ContainerPtr);
		return true;
	}

	return false;
}

bool USyReflectionLibrary::Generic_GetPropertyAsStruct(const UScriptStruct* StructDef, const void* ContainerPtr, FName PropertyName, const UScriptStruct* OutStructDef, void* OutStructPtr, UScriptStruct*& OutFoundStructType)
{
	OutFoundStructType = nullptr;

	if (!StructDef || !ContainerPtr || !OutStructDef || !OutStructPtr) return false;

	const FProperty* Property = StructDef->FindPropertyByName(PropertyName);
	if (!Property) return false;

	const FStructProperty* StructProp = CastField<FStructProperty>(Property);
	if (!StructProp) return false;

	// Output the found type
	OutFoundStructType = StructProp->Struct;

	// Strict check: The property's struct type must match the requested output struct type
	if (StructProp->Struct != OutStructDef)
	{
		if (!StructProp->Struct->IsChildOf(OutStructDef))
		{
			// Mismatch
			return false;
		}
	}

	const void* SrcPtr = StructProp->ContainerPtrToValuePtr<void>(ContainerPtr);
	
	// Copy data
	OutStructDef->CopyScriptStruct(OutStructPtr, SrcPtr);
	
	return true;
}
