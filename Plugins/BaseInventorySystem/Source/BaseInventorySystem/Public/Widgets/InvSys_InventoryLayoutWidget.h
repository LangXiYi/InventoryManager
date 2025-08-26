// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryWidget.h"
#include "InvSys_InventoryLayoutWidget.generated.h"

class UInvSys_InventoryWidgetSlot;

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryLayoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInvSys_InventoryWidgetSlot* FindTagSlot(FGameplayTag InSlotTag);

	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual UInvSys_InventoryWidgetSlot* AddWidget(UUserWidget* Widget, const FGameplayTag& Tag);

protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UInvSys_InventoryWidgetSlot*> InventoryWidgetMapping;
};
