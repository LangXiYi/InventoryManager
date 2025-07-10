// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryItemDefinition.h"
#include "UObject/Object.h"
#include "InvSys_InventoryItemInstance.generated.h"

class UInvSys_InventoryItemFragment;
class UGridInvSys_InventoryItemDefinition;

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UInvSys_InventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/**
	 * 如果在 ItemInstance 中定义了一个需要同步的属性，且该属性在 AddItemDefinition 时传入了该类型的属性
	 * 那么你就必须在你的子类中定义一个与该属性类型一致的 InitItemInstanceProps 函数。*/
	 void InitItemInstanceProps(const int32& Data) {}

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	TSubclassOf<UInvSys_InventoryItemDefinition> GetItemDefinition() const
	{
		return ItemDefinition;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UInvSys_InventoryItemFragment* FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDef);

	void SetItemUniqueID(FGuid Guid);

	const FGuid& GetItemUniqueID() const
	{
		return ItemUniqueID;
	}

	UFUNCTION(BlueprintPure)
	FText GetItemDisplayName() const
	{
		check(ItemDefinition);
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->GetItemDisplayName();
	}

protected:
	UPROPERTY(Replicated)
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(Replicated)
	FGuid ItemUniqueID = FGuid();
};
