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
	void AddWidget(UUserWidget* NewWidget, const FGameplayTag& InventoryTag);

	template<class T>
	T* FindInventoryWidget(FGameplayTag InventoryTag);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory HUD")
	UWidget* FindInventoryWidget(FGameplayTag InventoryTag, TSubclassOf<UWidget> WidgetClass);

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
};

template <class T>
T* UInvSys_InventoryHUD::FindInventoryWidget(FGameplayTag InventoryTag)
{
	if (InventoryTag.IsValid() == false)
	{
		return nullptr;
	}
	if (InventoryWidgetMapping.Contains(InventoryTag))
	{
		return (T*)InventoryWidgetMapping[InventoryTag];
	}
	FGameplayTag ParentInventoryTag = InventoryTag;
	return FindInventoryWidget<T>(ParentInventoryTag);
}