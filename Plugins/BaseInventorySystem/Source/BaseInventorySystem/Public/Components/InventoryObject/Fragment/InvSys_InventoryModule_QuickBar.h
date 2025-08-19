// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryModule.h"
#include "InvSys_InventoryModule_QuickBar.generated.h"

class UInvSys_InventoryItemInstance;

USTRUCT(BlueprintType)
struct FInvSys_QuickBarChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UInvSys_InventoryComponent* InvComp;

	UPROPERTY(BlueprintReadOnly)
	TArray<UInvSys_InventoryItemInstance*> QuickBarItems;
};

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryModule_QuickBar : public UInvSys_InventoryModule
{
	GENERATED_BODY()

public:
	UInvSys_InventoryModule_QuickBar();

	virtual void InitInventoryFragment(UObject* PreEditFragment) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory Module|Quick Bar")
	void UpdateQuickBarItemReference(UInvSys_InventoryItemInstance* ItemReference, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory Module|Quick Bar")
	UInvSys_InventoryItemInstance* FindInventoryItemInstance(int32 Index);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_QuickBarItemReferences)
	TArray<TWeakObjectPtr<UInvSys_InventoryItemInstance>> QuickBarItemReferences;
	UFUNCTION()
	void OnRep_QuickBarItemReferences();
};
