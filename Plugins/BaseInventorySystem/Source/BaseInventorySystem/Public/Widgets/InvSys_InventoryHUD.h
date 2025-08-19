// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Components/InvSys_InventorySlot.h"
#include "Widgets/InvSys_InventoryLayoutWidget.h"
#include "InvSys_InventoryHUD.generated.h"

class UInvSys_InventorySlot;
class UInvSys_InventoryItemInstance;
class UInvSys_InventoryItemActionPanel;
/**
 * 供库存系统使用，可与其他系统的HUD搭配使用
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	void AddWidget(UUserWidget* NewWidget, FGameplayTag InventoryTag);

	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	void RemoveWidget(const FGameplayTag& InventoryTag);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory HUD")
	UInvSys_InventorySlot* FindInventorySlot(FGameplayTag InventoryTag) const;

	template<class T>
	T* FindInventoryWidget(FGameplayTag InventoryTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory HUD", meta =(DeterminesOutputType = WidgetClass))
	UUserWidget* FindInventoryWidget(FGameplayTag InventoryTag, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory HUD", meta =(DeterminesOutputType = WidgetClass))
	UUserWidget* FindAndCreateInventoryWidget(FGameplayTag InventoryTag, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	void DisplayInventoryItemActionList(UInvSys_InventoryItemInstance* ItemInstance);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory HUD")
	TObjectPtr<UInvSys_InventoryItemActionPanel> ItemActionPanel;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory HUD")
	TSubclassOf<UInvSys_InventoryItemActionPanel> ItemActionPanelClass;

private:
	UPROPERTY()
	TMap<FGameplayTag, UInvSys_InventorySlot*> InventoryWidgetMapping;

	UPROPERTY()
	TSet<FGameplayTag> DefaultInventoryWidgets;
};

template <class T>
T* UInvSys_InventoryHUD::FindInventoryWidget(FGameplayTag InventoryTag) const
{
	UInvSys_InventorySlot* InventorySlot = FindInventorySlot(InventoryTag);
	if (InventorySlot)
	{
		return InventorySlot->GetInventorySlotChild<T>();
	}
	return nullptr;
}
