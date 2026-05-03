/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Threading.Tasks;
using T3Foundation.Mvvm;

namespace T3Foundation.Services.Navigation
{
  /// <summary>
  /// ViewModel-first navigation service. Manages the active ViewModel and back-stack.
  /// </summary>
  public interface IT3NavigationService
  {
    /// <summary>
    /// Navigate to a ViewModel of the given type, resolved from DI.
    /// </summary>
    void NavigateTo<TViewModel>() where TViewModel : T3ViewModelBase;

    /// <summary>
    /// Navigate to a ViewModel of the given type with a parameter.
    /// </summary>
    void NavigateTo<TViewModel>(object parameter) where TViewModel : T3ViewModelBase;

    /// <summary>
    /// Navigate back to the previous ViewModel.
    /// </summary>
    void GoBack();

    /// <summary>
    /// Whether back-navigation is possible.
    /// </summary>
    bool CanGoBack { get; }

    /// <summary>
    /// The currently active ViewModel.
    /// </summary>
    T3ViewModelBase? CurrentViewModel { get; }

    /// <summary>
    /// Raised after navigation completes with the new active ViewModel.
    /// </summary>
    event Action<T3ViewModelBase> OnNavigated;
  }
}
