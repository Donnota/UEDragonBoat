// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DragonBoatPawn.generated.h"

// 龙舟类型
UENUM(BlueprintType)
enum class EBoatType : uint8
{
	Player		UMETA(DisplayName = "Player"),
	AI_1		UMETA(DisplayName = "AI 1"),
	AI_2		UMETA(DisplayName = "AI 2")
};

/**
 * 龙舟 - 自动向前移动的Pawn
 */
UCLASS()
class DRAGONBOAT_API ADragonBoatPawn : public APawn
{
	GENERATED_BODY()

public:
	ADragonBoatPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== 组件 ==========

	// 网格体（龙舟模型）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* BoatMesh;

	// 浮动移动组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UFloatingPawnMovement* MovementComponent;

	// 摄像机（仅玩家龙舟使用）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* Camera;

	// 弹簧臂（摄像机支架）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArm;

	// ========== 属性 ==========

	// 龙舟类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boat Config")
	EBoatType BoatType;

	// 基础速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat Config")
	float BaseSpeed;

	// 当前速度（基础速度 + Buff）
	UPROPERTY(BlueprintReadOnly, Category = "Boat State")
	float CurrentSpeed;

	// 速度Buff（加速/减速的累积值）
	UPROPERTY(BlueprintReadOnly, Category = "Boat State")
	float SpeedBuff;

	// 是否已到达终点
	UPROPERTY(BlueprintReadOnly, Category = "Boat State")
	bool bReachedFinish;

	// 到达终点的排名（0表示未到达）
	UPROPERTY(BlueprintReadOnly, Category = "Boat State")
	int32 FinishRank;

	// ========== 公共接口 ==========

	// 初始化龙舟
	UFUNCTION(BlueprintCallable, Category = "Boat")
	void InitializeBoat(EBoatType InBoatType, bool bEnableCamera);

	// 添加速度Buff（正数加速，负数减速）
	UFUNCTION(BlueprintCallable, Category = "Boat")
	void AddSpeedBuff(float BuffAmount);

	// 清除所有速度Buff
	UFUNCTION(BlueprintCallable, Category = "Boat")
	void ClearSpeedBuff();

	// 标记为到达终点
	UFUNCTION(BlueprintCallable, Category = "Boat")
	void MarkFinished(int32 Rank);

	// 获取当前速度
	UFUNCTION(BlueprintPure, Category = "Boat")
	float GetCurrentSpeed() const { return CurrentSpeed; }

	// 获取龙舟类型
	UFUNCTION(BlueprintPure, Category = "Boat")
	EBoatType GetBoatType() const { return BoatType; }

private:
	// 更新速度
	void UpdateSpeed();
};
