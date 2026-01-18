Q:

FInstancedStruct该如何定义以支持编辑器反射下基于一个Tag来定义数据结构列表

A:

好的，这是一个非常具体且常见的编辑器扩展需求。标准 `UPROPERTY` 元数据无法直接实现“根据一个属性（`GameplayTag`）的值动态改变另一个属性（`FInstancedStruct`）的可选类型列表或默认类型”。我们需要借助 Unreal Engine 的编辑器自定义能力来实现这个效果。

核心思路是：当编辑器检测到 `GameplayTag` 属性发生变化时，我们查询该 Tag 关联的 `USyOperationParamSchemaMetadata`，获取期望的 `UScriptStruct*` 类型，然后更新对应的 `FInstancedStruct` 属性，强制它使用（或至少默认指向）这个正确的结构体类型。

以下是两种主要的实现方式，各有优劣：

**方式一：使用 `PostEditChangeProperty` (在 C++ 类或结构体中)**

这是相对简单、侵入性较低的方法，不需要创建单独的编辑器模块或自定义类。它在属性更改 *之后* 做出反应。

1.  **定义容器结构体/类**: 确保你的 `GameplayTag` 和 `FInstancedStruct` 位于同一个 C++ `USTRUCT` 或 `UCLASS` 中。

    ```cpp
    // In your SyOperation module or appropriate place
    #pragma once

    #include "CoreMinimal.h"
    #include "GameplayTagContainer.h"
    #include "InstancedStruct.h"
    #include "SyTaggedInstancedStruct.generated.h" // Example name

    USTRUCT(BlueprintType)
    struct SYOPERATION_API FSyTaggedInstancedStruct
    {
        GENERATED_BODY()

        // The Tag that defines the expected struct type for Parameters
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
        FGameplayTag DefiningTag;

        // The actual parameters, whose struct type should match the one defined by DefiningTag's metadata
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters", meta = (BaseStruct = "/Script/CoreUObject.Object")) // Keep BaseStruct generic initially
        FInstancedStruct Parameters;

        FSyTaggedInstancedStruct() = default;

    #if WITH_EDITOR
        // Function to handle property changes within this struct (if used directly in a UCLASS)
        // Or this logic needs to be in the PostEditChangeProperty of the UCLASS *containing* this struct.
        // See notes below.
        bool HandleTagChanged(const FPropertyChangedEvent& PropertyChangedEvent, UObject* OwnerObject);
    #endif
    };

    // Example Usage in a UClass (like an Action Definition or part of FSyOperation if FSyOperation is a USTRUCT in a UCLASS)
    UCLASS()
    class YOURGAME_API USyActionDefinition : public UObject
    {
        GENERATED_BODY()

    public:
        UPROPERTY(EditAnywhere, Category="Action")
        FSyTaggedInstancedStruct ActionParams;

    #if WITH_EDITOR
        virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    #endif
    };
    ```

2.  **实现 `PostEditChangeProperty`**: 在包含 `FSyTaggedInstancedStruct` 的 `UCLASS` 中（或者如果 `FSyTaggedInstancedStruct` 本身需要跨多个类重用，你可能需要一个更通用的处理方式，但 `UCLASS` 的 `PostEditChangeProperty` 是最直接的）。

    ```cpp
    #include "YourSyActionDefinition.h" // Include your class header
    #include "TagMetadata/Public/DS_TagMetadata.h" // Assuming UDS_TagMetadata is accessible
    #include "SyOperation/Public/USyOperationParamSchemaMetadata.h" // Include your Schema Metadata Class
    #include "InstancedStruct.h"
    #include "GameplayTagContainer.h"
    #include "Logging/LogMacros.h" // For logging

    // Define a log category if you don't have one
    // DECLARE_LOG_CATEGORY_EXTERN(LogSyEditor, Log, All);
    // DEFINE_LOG_CATEGORY(LogSyEditor);

    #if WITH_EDITOR
    void USyActionDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);

        const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
        const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

        // Check if the changed property is 'DefiningTag' within our 'ActionParams' struct
        if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USyActionDefinition, ActionParams) &&
            PropertyName == GET_MEMBER_NAME_CHECKED(FSyTaggedInstancedStruct, DefiningTag))
        {
            UE_LOG(LogTemp, Log, TEXT("DefiningTag changed for ActionParams. Updating FInstancedStruct type."));

            // 1. Get the new Tag value
            FGameplayTag NewTag = ActionParams.DefiningTag;

            // 2. Find the Schema Struct associated with this tag
            UScriptStruct* ExpectedStructType = nullptr;
            if (NewTag.IsValid())
            {
                USyOperationParamSchemaMetadata* SchemaMetadata = UDS_TagMetadata::GetTagMetadataByClass<USyOperationParamSchemaMetadata>(NewTag);
                if (SchemaMetadata && SchemaMetadata->ParameterStructType.IsValid()) // Use IsValid() for TSoftObjectPtr
                {
                    // Attempt to load the struct if it's not already loaded
                    SchemaMetadata->ParameterStructType.LoadSynchronous();
                    ExpectedStructType = Cast<UScriptStruct>(SchemaMetadata->ParameterStructType.Get());

                     if (!ExpectedStructType)
                     {
                         UE_LOG(LogTemp, Warning, TEXT("Tag '%s' has SchemaMetadata, but ParameterStructType '%s' could not be loaded or cast to UScriptStruct."),
                             *NewTag.ToString(), *SchemaMetadata->ParameterStructType.ToString());
                     }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Tag '%s' is valid but no valid USyOperationParamSchemaMetadata found or ParameterStructType is not set."), *NewTag.ToString());
                }
            }
             else
             {
                 UE_LOG(LogTemp, Log, TEXT("DefiningTag was cleared."));
                 // Optional: Clear the Parameters struct as well if the tag is invalid/none
                 // ActionParams.Parameters.Reset();
                 // return; // Exit early if tag is cleared
             }

            // 3. Get the current struct type in the FInstancedStruct
            UScriptStruct* CurrentStructType = ActionParams.Parameters.GetScriptStruct();

            // 4. If the expected type is valid and different from the current type (or current is None)
            if (ExpectedStructType != nullptr && ExpectedStructType != CurrentStructType)
            {
                UE_LOG(LogTemp, Log, TEXT("Updating Parameters struct type from '%s' to '%s' based on Tag '%s'."),
                    CurrentStructType ? *CurrentStructType->GetName() : TEXT("None"),
                    *ExpectedStructType->GetName(),
                    *NewTag.ToString());

                // Reset and initialize the FInstancedStruct with the new, correct type
                // This creates a default instance of the ExpectedStructType
                ActionParams.Parameters.InitializeAs(ExpectedStructType);

                // Notify the editor that this property also changed implicitly
                FPropertyChangedEvent ParamsChangedEvent(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(USyActionDefinition, ActionParams)));
                                                        // ^ Find the UPROPERTY FInstancedStruct member
                PostEditChangeProperty(ParamsChangedEvent); // Re-trigger PostEditChangeProperty for the Parameters struct itself to ensure UI updates
            }
            else if (ExpectedStructType == nullptr && CurrentStructType != nullptr)
            {
                 UE_LOG(LogTemp, Log, TEXT("Expected struct type is None based on Tag '%s', clearing current Parameters struct ('%s')."),
                       *NewTag.ToString(), *CurrentStructType->GetName());
                // If the tag defines no struct (or tag is invalid), clear the parameters
                ActionParams.Parameters.Reset();
                 FPropertyChangedEvent ParamsChangedEvent(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(USyActionDefinition, ActionParams)));
                 PostEditChangeProperty(ParamsChangedEvent);
            }
             else if (ExpectedStructType != nullptr && ExpectedStructType == CurrentStructType)
             {
                  UE_LOG(LogTemp, Log, TEXT("Parameters struct type '%s' already matches Tag '%s'. No change needed."),
                        *ExpectedStructType->GetName(), *NewTag.ToString());
             }
        }
    }
    #endif
    ```

*   **优点**:
    *   实现简单，代码量少。
    *   不需要单独的编辑器模块。
*   **缺点**:
    *   用户体验稍差：用户仍然可以在 `FInstancedStruct` 的下拉列表中选择 *任何* 结构体。只有当 `DefiningTag` 改变 *之后*，`Parameters` 字段才会被强制重置为正确的类型（丢失用户可能已输入的数据）。
    *   如果用户先设置了 `Parameters`，再改变 `DefiningTag`，数据会丢失。
    *   逻辑分散在每个使用 `FSyTaggedInstancedStruct` 的 `UCLASS` 中（除非设计更复杂的基类或辅助函数）。

**方式二：使用 `IDetailCustomization` (编辑器模块)**

这是更强大、用户体验更好的方法，但需要创建和注册编辑器自定义类。

1.  **创建编辑器模块**: 如果你的插件还没有编辑器模块，你需要创建一个（例如 `SyOperationEditor`）。
2.  **定义容器结构体**: 同方式一 (`FSyTaggedInstancedStruct`)。
3.  **创建 `IDetailCustomization` 类**:

    ```cpp
    // In your Editor Module (e.g., SyOperationEditor)
    #pragma once

    #include "IDetailCustomization.h"
    #include "PropertyHandle.h"
    #include "InstancedStruct.h"
    #include "GameplayTagContainer.h"

    class IDetailLayoutBuilder;
    class UScriptStruct;

    class FSyTaggedInstancedStructCustomization : public IDetailCustomization
    {
    public:
        static TSharedRef<IDetailCustomization> MakeInstance();

        // IDetailCustomization interface
        virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
        // End of IDetailCustomization interface

    private:
        TSharedPtr<IPropertyHandle> TagPropertyHandle;
        TSharedPtr<IPropertyHandle> ParametersPropertyHandle;
        IDetailLayoutBuilder* CachedDetailBuilder = nullptr; // Store builder for potential refresh

        void OnTagChanged();
        UScriptStruct* GetExpectedStructTypeFromTag(const FGameplayTag& Tag) const;
    };
    ```

4.  **实现 `IDetailCustomization`**:

    ```cpp
    #include "FSyTaggedInstancedStructCustomization.h"
    #include "DetailLayoutBuilder.h"
    #include "DetailCategoryBuilder.h"
    #include "DetailWidgetRow.h"
    #include "PropertyCustomizationHelpers.h" // For property widgets
    #include "SyTaggedInstancedStruct.h" // Your struct header
    #include "TagMetadata/Public/DS_TagMetadata.h"
    #include "SyOperation/Public/USyOperationParamSchemaMetadata.h"
    #include "Widgets/Input/SGameplayTagWidget.h" // If needed for custom tag widget
    #include "InstancedStructDetails.h" // For FInstancedStruct details customization reference

    #define LOCTEXT_NAMESPACE "SyTaggedInstancedStructCustomization"

    TSharedRef<IDetailCustomization> FSyTaggedInstancedStructCustomization::MakeInstance()
    {
        return MakeShareable(new FSyTaggedInstancedStructCustomization);
    }

    void FSyTaggedInstancedStructCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
    {
        CachedDetailBuilder = &DetailBuilder;

        // --- Get Handles to the properties within FSyTaggedInstancedStruct ---
        // IMPORTANT: This customization customizes the *container* object (e.g., USyActionDefinition)
        // So we need to get the property for the FSyTaggedInstancedStruct itself first.
        // Assuming we are customizing the UObject that *contains* FSyTaggedInstancedStruct
        // (Replace "ActionParams" with the actual UPROPERTY name)
        TSharedPtr<IPropertyHandle> StructPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USyActionDefinition, ActionParams)); // Example property name

        if (!StructPropertyHandle || !StructPropertyHandle->IsValidHandle()) return;

        // Now get the handles to the members *inside* FSyTaggedInstancedStruct
        TagPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSyTaggedInstancedStruct, DefiningTag));
        ParametersPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSyTaggedInstancedStruct, Parameters));

        if (!TagPropertyHandle || !TagPropertyHandle->IsValidHandle() || !ParametersPropertyHandle || !ParametersPropertyHandle->IsValidHandle()) return;

        // --- Customize Layout ---
        IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Parameters"); // Or get category from property handle

        // Add the DefiningTag property using default layout or custom widget
        Category.AddProperty(TagPropertyHandle);

        // Add the Parameters property using default layout
        // TODO: Advanced - Could potentially replace the widget with one that filters the struct picker based on the tag, but that's complex.
        // For now, we just show the default FInstancedStruct editor.
        Category.AddProperty(ParametersPropertyHandle);

        // --- Handle Tag Changes ---
        TagPropertyHandle->SetOnPropertyValueChanged(IDetailPropertyUtilities::FOnPropertyValueChanged::CreateSP(this, &FSyTaggedInstancedStructCustomization::OnTagChanged));

        // Initial check in case a tag is already set when the details panel opens
        OnTagChanged();
    }

    void FSyTaggedInstancedStructCustomization::OnTagChanged()
    {
        if (!TagPropertyHandle.IsValid() || !TagPropertyHandle->IsValidHandle() || !ParametersPropertyHandle.IsValid() || !ParametersPropertyHandle->IsValidHandle()) return;

        FGameplayTag CurrentTag;
        FPropertyAccess::Result Result = TagPropertyHandle->GetValue(CurrentTag);

        if (Result == FPropertyAccess::Success)
        {
            UScriptStruct* ExpectedStructType = GetExpectedStructTypeFromTag(CurrentTag);

            // Get current parameters struct data
            FInstancedStruct CurrentParams;
            Result = ParametersPropertyHandle->GetValue(CurrentParams); // Note: GetValue copies!

            if (Result == FPropertyAccess::Success)
            {
                 UScriptStruct* CurrentStructType = CurrentParams.GetScriptStruct();

                 if (ExpectedStructType != nullptr && ExpectedStructType != CurrentStructType)
                 {
                      UE_LOG(LogTemp, Log, TEXT("[Customization] Updating Parameters struct type from '%s' to '%s' based on Tag '%s'."),
                           CurrentStructType ? *CurrentStructType->GetName() : TEXT("None"),
                           *ExpectedStructType->GetName(),
                           *CurrentTag.ToString());

                      // Create a new default instance of the correct type
                      FInstancedStruct NewParams(ExpectedStructType);

                      // Set the value back to the property handle
                      // IMPORTANT: Use SetValue not GetValue/Modify!
                      ParametersPropertyHandle->SetValue(NewParams, EPropertyValueSetFlags::DefaultFlags);
                      // Optionally force refresh if UI doesn't update: CachedDetailBuilder->ForceRefreshDetails();
                 }
                 else if (ExpectedStructType == nullptr && CurrentStructType != nullptr)
                 {
                     UE_LOG(LogTemp, Log, TEXT("[Customization] Expected struct type is None based on Tag '%s', clearing current Parameters struct ('%s')."),
                           *CurrentTag.ToString(), *CurrentStructType->GetName());

                     ParametersPropertyHandle->SetValue(FInstancedStruct(), EPropertyValueSetFlags::DefaultFlags); // Set empty struct
                 }
            }
             else
             {
                 UE_LOG(LogTemp, Warning, TEXT("[Customization] Failed to get current Parameters value."));
             }
        }
         else
         {
             UE_LOG(LogTemp, Warning, TEXT("[Customization] Failed to get DefiningTag value."));
         }
    }

    UScriptStruct* FSyTaggedInstancedStructCustomization::GetExpectedStructTypeFromTag(const FGameplayTag& Tag) const
    {
        if (!Tag.IsValid()) return nullptr;

        USyOperationParamSchemaMetadata* SchemaMetadata = UDS_TagMetadata::GetTagMetadataByClass<USyOperationParamSchemaMetadata>(Tag);
        if (SchemaMetadata && SchemaMetadata->ParameterStructType.IsValid())
        {
            SchemaMetadata->ParameterStructType.LoadSynchronous(); // Ensure it's loaded
            return Cast<UScriptStruct>(SchemaMetadata->ParameterStructType.Get());
        }
        return nullptr;
    }

    #undef LOCTEXT_NAMESPACE
    ```

5.  **注册 Customization**: 在编辑器模块的 `StartupModule` 中注册。

    ```cpp
    #include "Modules/ModuleManager.h"
    #include "PropertyEditorModule.h"
    #include "SyTaggedInstancedStructCustomization.h" // Your customization header
    #include "SyTaggedInstancedStruct.h" // Your struct header
    #include "YourSyActionDefinition.h" // The UCLASS header

    class FSyOperationEditorModule : public IModuleInterface
    {
    public:
        virtual void StartupModule() override
        {
            FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

            // Register customization for the UCLASS containing the struct
             PropertyModule.RegisterCustomClassLayout(
                 USyActionDefinition::StaticClass()->GetFName(), // The name of the UClass we want to customize
                 FOnGetDetailCustomizationInstance::CreateStatic(&FSyTaggedInstancedStructCustomization::MakeInstance) // Your customization class factory
             );

            // OR, if you want to customize the FSyTaggedInstancedStruct *wherever it appears*
            // (More complex setup might be needed inside the customization to handle different outer objects)
            // PropertyModule.RegisterCustomPropertyTypeLayout(
            //    FSyTaggedInstancedStruct::StaticStruct()->GetFName(),
            //    FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSyTaggedInstancedStructCustomization::MakeInstance) // Needs IPropertyTypeCustomization base instead
            // );

            PropertyModule.NotifyCustomizationModuleChanged();
        }

        virtual void ShutdownModule() override
        {
            if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
            {
                FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
                PropertyModule.UnregisterCustomClassLayout(USyActionDefinition::StaticClass()->GetFName()); // Unregister matching the registration
                 // OR PropertyModule.UnregisterCustomPropertyTypeLayout(FSyTaggedInstancedStruct::StaticStruct()->GetFName());
            }
        }
    };

    IMPLEMENT_MODULE(FSyOperationEditorModule, SyOperationEditor); // Adjust module name
    ```

*   **优点**:
    *   用户体验好：当 Tag 改变时，`Parameters` 字段会自动重置为正确的结构体类型，用户无需猜测。
    *   逻辑集中：自定义逻辑位于专门的编辑器类中。
    *   更强大：可以进一步扩展以修改编辑器 UI（例如，禁用或过滤 `FInstancedStruct` 的类型选择器，但这相当复杂）。
*   **缺点**:
    *   实现复杂：需要创建和管理编辑器模块和自定义类。
    *   需要对 Unreal Editor 的细节系统有更深入的了解。
    *   注册/反注册逻辑需要正确处理。

**总结与建议:**

*   **从 `PostEditChangeProperty` 开始**: 这是最快实现基本功能的方法。先尝试这种方式，看看它是否满足你的需求。
*   **如果需要更好的 UX，再考虑 `IDetailCustomization`**: 如果发现 `PostEditChangeProperty` 的 UX 问题（数据丢失、用户可选错类型）无法接受，那么投入时间实现 `IDetailCustomization` 是值得的。
*   **`TagMetadata` 查询**: 确保你的 `TagMetadata` 查询逻辑 (`UDS_TagMetadata::GetTagMetadataByClass...`) 是健壮的，并能正确加载 `TSoftObjectPtr<UScriptStruct>`。
*   **错误处理**: 在代码中添加日志和空指针检查，以处理 Tag 没有关联 Schema 或 Schema 无法加载的情况。

选择哪种方式取决于你对编辑器体验的要求和愿意投入的开发时间。

尝试为我分析在编辑器下为任意Tag携带匹配的预定义数据结构的上述两种方法的可行性？
