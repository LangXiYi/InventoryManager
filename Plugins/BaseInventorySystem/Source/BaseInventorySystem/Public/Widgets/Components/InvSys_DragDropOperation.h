// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InvSys_DragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_DragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Dragged_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;
};
