/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using T3Foundation.Mvvm;
using T3Foundation.Services.DI;

namespace T3Foundation.Services.Navigation
{
  /// <summary>
  /// Default ViewModel-first navigation implementation.
  /// Resolves ViewModels from the DI container and manages lifecycle hooks.
  /// </summary>
  public class T3NavigationService : T3ObservableObject, IT3NavigationService
  {
    private readonly Stack<T3ViewModelBase> _backStack = new();
    private T3ViewModelBase? _currentViewModel;

    /// <inheritdoc/>
    public T3ViewModelBase? CurrentViewModel
    {
      get => _currentViewModel;
      private set => SetProperty(ref _currentViewModel, value);
    }

    /// <inheritdoc/>
    public bool CanGoBack => _backStack.Count > 0;

    /// <inheritdoc/>
    public event Action<T3ViewModelBase>? OnNavigated;

    /// <inheritdoc/>
    public void NavigateTo<TViewModel>() where TViewModel : T3ViewModelBase
    {
      NavigateTo<TViewModel>(null!);
    }

    /// <inheritdoc/>
    public void NavigateTo<TViewModel>(object parameter) where TViewModel : T3ViewModelBase
    {
      var vm = T3ServiceCollection.Resolve<TViewModel>();
      PerformNavigation(vm);
    }

    /// <inheritdoc/>
    public void GoBack()
    {
      if (!CanGoBack)
        return;

      var previous = _backStack.Pop();
      if (_currentViewModel != null)
        _currentViewModel.OnDeactivated();

      CurrentViewModel = previous;
      previous.OnActivated();
      OnNavigated?.Invoke(previous);

      Log($"Navigated back to {previous.GetType().Name}", T3LogLevel.Debug);
    }

    private void PerformNavigation(T3ViewModelBase newViewModel)
    {
      if (_currentViewModel != null)
      {
        if (!_currentViewModel.CanClose())
        {
          Log($"Navigation blocked by {_currentViewModel.GetType().Name}.CanClose()", T3LogLevel.Debug);
          return;
        }

        _currentViewModel.OnDeactivated();
        _backStack.Push(_currentViewModel);
      }

      CurrentViewModel = newViewModel;

      if (!newViewModel.IsInitialized)
        newViewModel.InitializeAsync().FireAndForget();

      newViewModel.OnActivated();
      OnNavigated?.Invoke(newViewModel);

      Log($"Navigated to {newViewModel.GetType().Name}", T3LogLevel.Debug);
    }
  }
}
