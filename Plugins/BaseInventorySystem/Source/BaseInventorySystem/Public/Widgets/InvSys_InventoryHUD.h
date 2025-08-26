// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_CommonType.h"
#include "Blueprint/UserWidget.h"
#include "Components/InvSys_InventoryWidgetSlot.h"
#include "InvSys_InventoryHUD.generated.h"

class UInvSys_InventoryWidgetSlot;
class UInvSys_InventoryItemInstance;
class UInvSys_InventoryItemActionPanel;

/**
 * 供库存系统使用，可与其他系统的HUD搭配使用
 * NOTE：由于使用了 World 创建控件，所以会导致 NativeOnInitialized 函数不执行
 * todo::需要更好的UI框架管理控件
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UInvSys_InventoryHUD(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	bool AddWidget(UUserWidget* NewWidget, FGameplayTag InventoryTag);

	/**
	 * 移除指定标签下的所有控件
	 * 注意：不会移除对应标签的插槽以及 LayoutWidget
	 * @param InventoryTag 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	void RemoveWidget(FGameplayTag InventoryTag);

	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	void SetInventoryWidgetVisibility(FGameplayTag InventoryTag, ESlateVisibility InVisibility);

	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	void SetInventorySlotVisibility(FGameplayTag InventoryTag, ESlateVisibility InVisibility);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory HUD")
	UInvSys_InventoryWidgetSlot* FindInventorySlot(FGameplayTag InventoryTag) const;

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
	TMap<FGameplayTag, UInvSys_InventoryWidgetSlot*> InventoryWidgetMapping;

	UPROPERTY()
	TSet<FGameplayTag> DefaultInventoryWidgets;
};

template <class T>
T* UInvSys_InventoryHUD::FindInventoryWidget(FGameplayTag InventoryTag) const
{
	UInvSys_InventoryWidgetSlot* InventorySlot = FindInventorySlot(InventoryTag);
	if (InventorySlot)
	{
		return InventorySlot->GetInventorySlotChild<T>();
	}
	return nullptr;
}
